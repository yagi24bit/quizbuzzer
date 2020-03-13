#define PIN_LATCH_INV 2
#define PIN_LATCH     3
#define PIN_CLOCK     4
#define PIN_INPUT     5
#define PIN_OUTPUT    6
#define PIN_RESET     8

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

void setup() {
	Serial.begin(9600);

	pinMode(PIN_LATCH_INV, OUTPUT); // LATCH の反転
	pinMode(PIN_LATCH, OUTPUT); // LATCH
	pinMode(PIN_CLOCK, OUTPUT); // CLOCK
	pinMode(PIN_INPUT, INPUT_PULLUP); // ボタン入力
	pinMode(PIN_OUTPUT, OUTPUT); // LED
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
	int times = 0;
	unsigned long m = millis();

	while(1) {
		unsigned short input_buttons_prev = input_buttons;
		input_buttons = shiftIO(output_buttons) & 0xFF; // 暫定、とりあえず下 8 桁だけ

		if(input_buttons  != input_buttons_prev) {
			Serial.println(input_buttons | 0b10000000000000000, BIN);
			if(output_buttons == 0) { output_buttons = input_buttons; }
		}

		// リセット
		if(digitalRead(PIN_RESET) == LOW) { output_buttons = 0; }

		// 処理速度計測 (暫定)
		times++;
		if(millis() - m > 1000) {
			m = millis();
			Serial.println(times);
			times = 0;
		}
	}
}
