#define PIN_LATCH_INV   2
#define PIN_LATCH       3
#define PIN_CLOCK       4
#define PIN_INPUT       5
#define PIN_OUTPUT      6

#define PIN_START       8   // B0
#define PIN_CORRECT     9   // B1
#define PIN_INCORRECT   10  // B2
#define PIN_RESET       11  // B3

inline unsigned short rand_next(unsigned short rand) {
	// 乱数もどき
	return (rand >> 15) | (rand << 1);
}

unsigned short shiftIO(unsigned short output) {
	unsigned short input = 0;

	// LATCH 送信 (74HC165)
	digitalWrite(PIN_LATCH_INV, LOW);
	digitalWrite(PIN_LATCH_INV, HIGH);

	for(int i = 0; i < 16; i++) {
		input = input | (digitalRead(PIN_INPUT) << i);
		digitalWrite(PIN_OUTPUT, (output >> i) & 1);

		// CLOCK 送信
		digitalWrite(PIN_CLOCK, HIGH);
		digitalWrite(PIN_CLOCK, LOW);
	}

	// LATCH 送信 (74HC595)
	digitalWrite(PIN_LATCH, HIGH);
	digitalWrite(PIN_LATCH, LOW);

	return ~input;
}

void playSound() {
}

void setup() {
	Serial.begin(9600);

	pinMode(PIN_LATCH_INV, OUTPUT); // LATCH の反転
	pinMode(PIN_LATCH, OUTPUT); // LATCH
	pinMode(PIN_CLOCK, OUTPUT); // CLOCK
	pinMode(PIN_INPUT, INPUT_PULLUP); // ボタン入力
	pinMode(PIN_OUTPUT, OUTPUT); // LED

	pinMode(PIN_START, INPUT_PULLUP); // 出題
	pinMode(PIN_CORRECT, INPUT_PULLUP); // 正解
	pinMode(PIN_INCORRECT, INPUT_PULLUP); // 不正解
	pinMode(PIN_RESET, INPUT_PULLUP); // リセット

	digitalWrite(PIN_LATCH_INV, HIGH);
	digitalWrite(PIN_LATCH, LOW);
	digitalWrite(PIN_CLOCK, LOW);
	shiftIO(0);

	Serial.println("start");
}

void loop() {
	unsigned short input_buttons = 0;
	unsigned short output_buttons = 0;
	unsigned short rand = 1;
	unsigned long m_sound = millis();

	int times = 0;
	unsigned long m = millis();

	while(1) {
		unsigned short input_buttons_prev = input_buttons;
		input_buttons = shiftIO(output_buttons) & 0xFF; // 暫定、とりあえず下 8 桁だけ
		rand = rand_next(rand);

		if(millis() - m_sound > 500) {
			// 出題者ボタン
			unsigned short operator_buttons = ~PINB & 0xF;
			if(operator_buttons) {
				Serial.println(operator_buttons | 0b10000, BIN);
				output_buttons = 0;
				playSound();
				m_sound = millis();
				continue;
			}
		}

		// 回答者ボタン
		unsigned short input_buttons_d = input_buttons & ~input_buttons_prev; // ボタンが押された瞬間のみ検知
		if(output_buttons == 0 && input_buttons_d) {
			playSound();
			m_sound = millis();
			do {
				output_buttons = input_buttons_d & rand; // 複数のボタンが押されている場合はどれか 1 つに絞る
				rand = rand_next(rand);
			} while(output_buttons == 0);
		}

		// 以下、デバッグメッセージ
		// 回答者ボタン
		if(input_buttons != input_buttons_prev) {
			Serial.println(input_buttons | 0b10000000000000000, BIN);
		}
		// 処理速度計測 (暫定)
		times++;
		if(millis() - m > 1000) {
			m = millis();
			Serial.println(times);
			times = 0;
		}
	}
}
