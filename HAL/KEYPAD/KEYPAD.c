/**
 * @file KEYPAD.c
 * @brief Implementation of direct-button and 3x3 matrix keypad scanning.
 */
#include "KEYPAD.h"

/*
 * This driver can work in two modes:
 *
 * 1. KEYPAD_DIRECT_MODE:
 *    Three normal push buttons are connected directly to input pins.
 *
 * 2. Matrix mode:
 *    A 3x3 keypad is connected as 3 rows and 3 columns.
 *
 * Both modes use the same debounce state machine. The only difference is how
 * the raw pressed/released state is produced.
 */

/** @brief Persistent debounce state belonging to one physical key. */
typedef struct {
    /**
     * @brief Latest raw sample.
     * last_read:
     * The most recent raw reading seen from the button/key.
     *
     * This can still be noisy because a mechanical switch bounces when it is
     * pressed or released.
     */
    uint8_t last_read;

    /**
     * @brief Accepted debounced electrical state.
     * stable_state:
     * The accepted, debounced state.
     *
     * A key event is generated only when the raw state stays unchanged for
     * KEYPAD_DEBOUNCE_COUNT checks and then differs from this saved state.
     */
    uint8_t stable_state;

    /**
     * @brief Count of matching samples since the last change.
     * debounce_count:
     * Counts how many consecutive times the raw state matched last_read.
     *
     * If the raw state changes, this counter is reset because the signal is not
     * considered stable yet.
     */
    uint8_t debounce_count;
} KEYPAD_ButtonState_t;

/**
 * @brief Advance one key's debounce state machine by one raw sample.
 * @param current_state Current active-low electrical state.
 * @param button Persistent state for this physical key.
 * @param key Logical key identifier returned after a valid press.
 * @return key on a newly accepted press, otherwise KEYPAD_NO_KEY.
 */
static KEYPAD_switch_t KEYPAD_checkButtonState(
    uint8_t current_state,
    KEYPAD_ButtonState_t *button,
    KEYPAD_switch_t key
);

#ifdef KEYPAD_DIRECT_MODE
/** @brief Read and debounce one direct button input. */
static KEYPAD_switch_t KEYPAD_checkButton(
    volatile uint8_t *pin_reg,
    GPIO_PIN_t pin_no,
    KEYPAD_ButtonState_t *button,
    KEYPAD_switch_t key
);
#else
/** @brief Locate and debounce the active row within one matrix column. */
static KEYPAD_switch_t KEYPAD_checkRows(
    volatile uint8_t *col,
    GPIO_PIN_t pin_no,
    KEYPAD_switch_t SW_row1,
    KEYPAD_switch_t SW_row2,
    KEYPAD_switch_t SW_row3,
    KEYPAD_ButtonState_t keyState[3]
);

/** @brief Feed released samples to all three keys in one inactive column. */
static void KEYPAD_releaseRows(
    KEYPAD_ButtonState_t keyState[3],
    KEYPAD_switch_t SW_row1,
    KEYPAD_switch_t SW_row2,
    KEYPAD_switch_t SW_row3
);
#endif

/** @copydoc KEYPAD_init */
void KEYPAD_init(KEYPAD_Pull_t pullup)
{
#ifdef KEYPAD_DIRECT_MODE
    /*
     * Direct mode:
     * Each button is an independent input pin. With pull-ups enabled, an
     * unpressed button reads HIGH and a pressed button reads LOW.
     */
    GPIO_Pin_Direction(&KEYPAD_BUTTON_1_DDR, KEYPAD_BUTTON_1_PIN_NO, GPIO_INPUT);
    GPIO_Pin_Direction(&KEYPAD_BUTTON_2_DDR, KEYPAD_BUTTON_2_PIN_NO, GPIO_INPUT);
    GPIO_Pin_Direction(&KEYPAD_BUTTON_3_DDR, KEYPAD_BUTTON_3_PIN_NO, GPIO_INPUT);

    if(pullup == KEYPAD_INTERNAL_PULLUP) {
        GPIO_Set_PullUp(&KEYPAD_BUTTON_1_PORT, KEYPAD_BUTTON_1_PIN_NO);
        GPIO_Set_PullUp(&KEYPAD_BUTTON_2_PORT, KEYPAD_BUTTON_2_PIN_NO);
        GPIO_Set_PullUp(&KEYPAD_BUTTON_3_PORT, KEYPAD_BUTTON_3_PIN_NO);
    }
#else
    /*
     * Matrix mode:
     * Rows are outputs. Columns are inputs with pull-ups.
     *
     * Normal idle condition:
     * - Rows are driven LOW.
     * - Columns are pulled HIGH.
     *
     * If a key is pressed while rows are LOW, the matching column becomes LOW.
     * That tells KEYPAD_getKey() that this column contains a pressed key.
     */
    GPIO_Pin_Direction(&KEYPAD_ROW_1_DDR, KEYPAD_ROW_1_PIN_NO, GPIO_OUTPUT);
    GPIO_Pin_Direction(&KEYPAD_ROW_2_DDR, KEYPAD_ROW_2_PIN_NO, GPIO_OUTPUT);
    GPIO_Pin_Direction(&KEYPAD_ROW_3_DDR, KEYPAD_ROW_3_PIN_NO, GPIO_OUTPUT);

    GPIO_Pin_Write(&KEYPAD_ROW_1_PORT, KEYPAD_ROW_1_PIN_NO, GPIO_LOW);
    GPIO_Pin_Write(&KEYPAD_ROW_2_PORT, KEYPAD_ROW_2_PIN_NO, GPIO_LOW);
    GPIO_Pin_Write(&KEYPAD_ROW_3_PORT, KEYPAD_ROW_3_PIN_NO, GPIO_LOW);

    GPIO_Pin_Direction(&KEYPAD_COL_1_DDR, KEYPAD_COL_1_PIN_NO, GPIO_INPUT);
    GPIO_Pin_Direction(&KEYPAD_COL_2_DDR, KEYPAD_COL_2_PIN_NO, GPIO_INPUT);
    GPIO_Pin_Direction(&KEYPAD_COL_3_DDR, KEYPAD_COL_3_PIN_NO, GPIO_INPUT);

    if(pullup == KEYPAD_INTERNAL_PULLUP) {
        GPIO_Set_PullUp(&KEYPAD_COL_1_PORT, KEYPAD_COL_1_PIN_NO);
        GPIO_Set_PullUp(&KEYPAD_COL_2_PORT, KEYPAD_COL_2_PIN_NO);
        GPIO_Set_PullUp(&KEYPAD_COL_3_PORT, KEYPAD_COL_3_PIN_NO);
    }
#endif
}

/** @copydoc KEYPAD_getKey */
KEYPAD_switch_t KEYPAD_getKey(void)
{
#ifdef KEYPAD_DIRECT_MODE
    /*
     * Direct mode keeps one debounce state per physical button.
     *
     * Static is important here: debounce state must survive between function
     * calls. If these variables were automatic/local without static, debounce
     * would restart from zero every time KEYPAD_getKey() is called.
     */
    static KEYPAD_ButtonState_t button1_state = {KEYPAD_RELEASED, KEYPAD_RELEASED, 0};
    static KEYPAD_ButtonState_t button2_state = {KEYPAD_RELEASED, KEYPAD_RELEASED, 0};
    static KEYPAD_ButtonState_t button3_state = {KEYPAD_RELEASED, KEYPAD_RELEASED, 0};

    KEYPAD_switch_t key;

    key = KEYPAD_checkButton(
        &KEYPAD_BUTTON_1_PIN,
        KEYPAD_BUTTON_1_PIN_NO,
        &button1_state,
        KEYPAD_SW1
    );
    if(key != KEYPAD_NO_KEY) {
        return key;
    }

    key = KEYPAD_checkButton(
        &KEYPAD_BUTTON_2_PIN,
        KEYPAD_BUTTON_2_PIN_NO,
        &button2_state,
        KEYPAD_SW2
    );
    if(key != KEYPAD_NO_KEY) {
        return key;
    }

    key = KEYPAD_checkButton(
        &KEYPAD_BUTTON_3_PIN,
        KEYPAD_BUTTON_3_PIN_NO,
        &button3_state,
        KEYPAD_SW3
    );
    if(key != KEYPAD_NO_KEY) {
        return key;
    }

    return KEYPAD_NO_KEY;
#else
    /*
     * key_state[column][row]
     *
     * The matrix has 9 physical keys, so every key gets its own debounce state.
     * This matters because each key can bounce independently.
     *
     * Column 0 contains SW1, SW4, SW7.
     * Column 1 contains SW2, SW5, SW8.
     * Column 2 contains SW3, SW6, SW9.
     */
    static KEYPAD_ButtonState_t key_state[3][3] = {
        {
            {KEYPAD_RELEASED, KEYPAD_RELEASED, 0},
            {KEYPAD_RELEASED, KEYPAD_RELEASED, 0},
            {KEYPAD_RELEASED, KEYPAD_RELEASED, 0}
        },
        {
            {KEYPAD_RELEASED, KEYPAD_RELEASED, 0},
            {KEYPAD_RELEASED, KEYPAD_RELEASED, 0},
            {KEYPAD_RELEASED, KEYPAD_RELEASED, 0}
        },
        {
            {KEYPAD_RELEASED, KEYPAD_RELEASED, 0},
            {KEYPAD_RELEASED, KEYPAD_RELEASED, 0},
            {KEYPAD_RELEASED, KEYPAD_RELEASED, 0}
        }
    };

    /*
     * First stage of matrix scan:
     * Check which column is pulled LOW while all rows are LOW.
     *
     * If column 1 is LOW, one of SW1/SW4/SW7 is pressed.
     * If column 2 is LOW, one of SW2/SW5/SW8 is pressed.
     * If column 3 is LOW, one of SW3/SW6/SW9 is pressed.
     *
     * After finding the active column, KEYPAD_checkRows() drives rows HIGH one
     * by one to discover which row is connected through the pressed key.
     */
    if(GPIO_Pin_Read(&KEYPAD_COL_1_PIN, KEYPAD_COL_1_PIN_NO) == GPIO_LOW) {
        return KEYPAD_checkRows(
            &KEYPAD_COL_1_PIN,
            KEYPAD_COL_1_PIN_NO,
            KEYPAD_SW1,
            KEYPAD_SW4,
            KEYPAD_SW7,
            key_state[0]
        );
    } else if(GPIO_Pin_Read(&KEYPAD_COL_2_PIN, KEYPAD_COL_2_PIN_NO) == GPIO_LOW) {
        return KEYPAD_checkRows(
            &KEYPAD_COL_2_PIN,
            KEYPAD_COL_2_PIN_NO,
            KEYPAD_SW2,
            KEYPAD_SW5,
            KEYPAD_SW8,
            key_state[1]
        );
    } else if(GPIO_Pin_Read(&KEYPAD_COL_3_PIN, KEYPAD_COL_3_PIN_NO) == GPIO_LOW) {
        return KEYPAD_checkRows(
            &KEYPAD_COL_3_PIN,
            KEYPAD_COL_3_PIN_NO,
            KEYPAD_SW3,
            KEYPAD_SW6,
            KEYPAD_SW9,
            key_state[2]
        );
    } else {
        /*
         * No column is active, so no key is currently pressed.
         *
         * We still must feed RELEASED into every key's debounce state. Without
         * this, a key could remain stored as "pressed" forever and the next
         * press of the same key would not generate a new event.
         */
        KEYPAD_releaseRows(key_state[0], KEYPAD_SW1, KEYPAD_SW4, KEYPAD_SW7);
        KEYPAD_releaseRows(key_state[1], KEYPAD_SW2, KEYPAD_SW5, KEYPAD_SW8);
        KEYPAD_releaseRows(key_state[2], KEYPAD_SW3, KEYPAD_SW6, KEYPAD_SW9);

        return KEYPAD_NO_KEY;
    }
#endif
}

#ifndef KEYPAD_DIRECT_MODE
/**
 * @brief Resolve which row is active in a column known to contain a press.
 * @param col Column PIN register address.
 * @param pin_no Column bit index.
 * @param SW_row1 Logical key for row 1.
 * @param SW_row2 Logical key for row 2.
 * @param SW_row3 Logical key for row 3.
 * @param keyState Three persistent states indexed by row.
 * @details Raises one row at a time, samples the column, and restores LOW.
 * @return A newly debounced key press, or KEYPAD_NO_KEY.
 */
static KEYPAD_switch_t KEYPAD_checkRows(
    volatile uint8_t *col,
    GPIO_PIN_t pin_no,
    KEYPAD_switch_t SW_row1,
    KEYPAD_switch_t SW_row2,
    KEYPAD_switch_t SW_row3,
    KEYPAD_ButtonState_t keyState[3]
)
{
    /*
     * Second stage of matrix scan:
     *
     * We already know this column is active. Now each row is driven HIGH for a
     * moment. The row connected to the pressed key will push the same column
     * HIGH, so that row is the key's row.
     *
     * keyState[0] is the debounce state for this column's row 1 key.
     * keyState[1] is the debounce state for this column's row 2 key.
     * keyState[2] is the debounce state for this column's row 3 key.
     */

    GPIO_Pin_Write(&KEYPAD_ROW_1_PORT, KEYPAD_ROW_1_PIN_NO, GPIO_HIGH);
    if(GPIO_Pin_Read(col, pin_no) == GPIO_HIGH) {
        GPIO_Pin_Write(&KEYPAD_ROW_1_PORT, KEYPAD_ROW_1_PIN_NO, GPIO_LOW);
        return KEYPAD_checkButtonState(KEYPAD_PRESSED, &keyState[0], SW_row1);
    }
    GPIO_Pin_Write(&KEYPAD_ROW_1_PORT, KEYPAD_ROW_1_PIN_NO, GPIO_LOW);
    KEYPAD_checkButtonState(KEYPAD_RELEASED, &keyState[0], SW_row1);

    GPIO_Pin_Write(&KEYPAD_ROW_2_PORT, KEYPAD_ROW_2_PIN_NO, GPIO_HIGH);
    if(GPIO_Pin_Read(col, pin_no) == GPIO_HIGH) {
        GPIO_Pin_Write(&KEYPAD_ROW_2_PORT, KEYPAD_ROW_2_PIN_NO, GPIO_LOW);
        return KEYPAD_checkButtonState(KEYPAD_PRESSED, &keyState[1], SW_row2);
    }
    GPIO_Pin_Write(&KEYPAD_ROW_2_PORT, KEYPAD_ROW_2_PIN_NO, GPIO_LOW);
    KEYPAD_checkButtonState(KEYPAD_RELEASED, &keyState[1], SW_row2);

    GPIO_Pin_Write(&KEYPAD_ROW_3_PORT, KEYPAD_ROW_3_PIN_NO, GPIO_HIGH);
    if(GPIO_Pin_Read(col, pin_no) == GPIO_HIGH) {
        GPIO_Pin_Write(&KEYPAD_ROW_3_PORT, KEYPAD_ROW_3_PIN_NO, GPIO_LOW);
        return KEYPAD_checkButtonState(KEYPAD_PRESSED, &keyState[2], SW_row3);
    }
    GPIO_Pin_Write(&KEYPAD_ROW_3_PORT, KEYPAD_ROW_3_PIN_NO, GPIO_LOW);
    KEYPAD_checkButtonState(KEYPAD_RELEASED, &keyState[2], SW_row3);

    return KEYPAD_NO_KEY;
}
#endif

#ifdef KEYPAD_DIRECT_MODE
/**
 * @brief Sample a direct input and pass it through the shared debouncer.
 * @param pin_reg Input register address.
 * @param pin_no Input bit index.
 * @param button Persistent state to update.
 * @param key Key identifier to report on a new press.
 * @return A newly debounced key press, or KEYPAD_NO_KEY.
 */
static KEYPAD_switch_t KEYPAD_checkButton(
    volatile uint8_t *pin_reg,
    GPIO_PIN_t pin_no,
    KEYPAD_ButtonState_t *button,
    KEYPAD_switch_t key
)
{
    /*
     * Direct buttons already give us the raw electrical state from one pin.
     * The shared debounce helper converts that raw state into a clean key event.
     */
    return KEYPAD_checkButtonState(GPIO_Pin_Read(pin_reg, pin_no), button, key);
}
#endif

/**
 * @brief Debounce one raw key state and report press transitions only.
 * @return The supplied key on a stable new press, otherwise KEYPAD_NO_KEY.
 */
static KEYPAD_switch_t KEYPAD_checkButtonState(
    uint8_t current_state,
    KEYPAD_ButtonState_t *button,
    KEYPAD_switch_t key
)
{
    /*
     * Debounce step 1:
     * If the raw reading changed, remember the new raw value but do not accept
     * it yet. A bouncing key can rapidly jump between HIGH and LOW.
     */
    if(current_state != button->last_read) {
        button->last_read = current_state;
        button->debounce_count = 0;
        return KEYPAD_NO_KEY;
    }

    /*
     * Debounce step 2:
     * The raw reading matched last_read. Count this as one stable sample.
     * Until enough equal samples are seen, no event is reported.
     */
    if(button->debounce_count < KEYPAD_DEBOUNCE_COUNT) {
        button->debounce_count++;
        return KEYPAD_NO_KEY;
    }

    /*
     * Debounce step 3:
     * The raw reading is stable, but it is the same as the already accepted
     * stable state. That means there is no new press/release transition.
     */
    if(current_state == button->stable_state) {
        return KEYPAD_NO_KEY;
    }

    /*
     * Debounce step 4:
     * Accept the new stable state. Only presses are reported to the application.
     * Releases are still saved so the next press can be detected later.
     */
    button->stable_state = current_state;

    if(current_state == KEYPAD_PRESSED) {
        return key;
    }

    return KEYPAD_NO_KEY;
}

#ifndef KEYPAD_DIRECT_MODE
/**
 * @brief Update release state for every key in one inactive matrix column.
 * @param keyState Three persistent states indexed by row.
 * @param SW_row1 Row 1 key identifier.
 * @param SW_row2 Row 2 key identifier.
 * @param SW_row3 Row 3 key identifier.
 * @details Discards return values because only release transitions are fed.
 */
static void KEYPAD_releaseRows(
    KEYPAD_ButtonState_t keyState[3],
    KEYPAD_switch_t SW_row1,
    KEYPAD_switch_t SW_row2,
    KEYPAD_switch_t SW_row3
)
{
    /*
     * Feed RELEASED into all three debounce states for one column.
     *
     * The returned value is intentionally ignored because this helper is used
     * only to update saved release state, not to report a key to the user.
     */
    KEYPAD_checkButtonState(KEYPAD_RELEASED, &keyState[0], SW_row1);
    KEYPAD_checkButtonState(KEYPAD_RELEASED, &keyState[1], SW_row2);
    KEYPAD_checkButtonState(KEYPAD_RELEASED, &keyState[2], SW_row3);
}
#endif
