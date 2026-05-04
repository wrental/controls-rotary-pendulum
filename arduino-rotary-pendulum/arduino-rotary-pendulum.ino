/* name: arduino-rotary-pendulum.ino
 * date: 04-24-2026
 * auth: wrental
 * desc: arduino implementation of rotary pendulum.
 */

// motor pin definitions
#define MOTOR_DIR (GPIO_NUM_1)
#define MOTOR_STEP (GPIO_NUM_2)
#define MOTOR_SLEEP (GPIO_NUM_3)
#define MOTOR_RESET (GPIO_NUM_4)
#define MOTOR_ENABLE (GPIO_NUM_5)

// encoder pin definitions
#define ENCODER_A (GPIO_NUM_17)
#define ENCODER_B (GPIO_NUM_18)

// global constants
#define ENCODER_RES 2400  // ticks/rotation
#define MOTOR_RES 3200    // ticks/rotation
#define SS_ERROR 3        // steady-state error margin (ticks)

// data sharing between threads
typedef struct pendulum_data_t {
  int loop_duration;
  int encoder_pos_ticks;
  int motor_pos_ticks;
};

pendulum_data_t pendulum_data_send;
int queue_item_size = sizeof(pendulum_data_send);
QueueHandle_t pendulum_data_queue = xQueueCreate(2, queue_item_size);

// global variables
int loop_start_time = 1;  // microseconds, placeholder
int loop_duration = 1;    // microseconds, placeholder
float loop_duration_sec = 0;
// encoder:
int encoder_pos_ticks = 0;  // cw = positive, ccw = negative
// motor:
uint8_t motor_dir = 0;    // 0 = cw, 1 = ccw from TOP VIEW
int motor_pos_ticks = 0;  // cw = positive, ccw = negative

// encoder interrupt service routines
void encoder_A_interrupt(void) {
  if (digitalRead(ENCODER_B) != digitalRead(ENCODER_A)) {
    encoder_pos_ticks++;
  } else {
    encoder_pos_ticks--;
  }
}
void encoder_B_interrupt(void) {
  if (digitalRead(ENCODER_B) == digitalRead(ENCODER_A)) {
    encoder_pos_ticks++;
  } else {
    encoder_pos_ticks--;
  }
}

// set pins, attach interrupts for encoder
void encoder_init(void) {
  // set pins for encoder
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);

  // set interrupt for encoder
  attachInterrupt(ENCODER_A, encoder_A_interrupt, CHANGE);
  attachInterrupt(ENCODER_B, encoder_B_interrupt, CHANGE);
}

// initialize stepper motor
void motor_init(void) {
  // disable motor output
  pinMode(MOTOR_ENABLE, OUTPUT);
  digitalWrite(MOTOR_ENABLE, HIGH);
  delay(1);

  // define output pins, states
  pinMode(MOTOR_DIR, OUTPUT);
  digitalWrite(MOTOR_DIR, LOW);
  delay(1);
  pinMode(MOTOR_STEP, OUTPUT);
  digitalWrite(MOTOR_STEP, LOW);
  delay(1);
  pinMode(MOTOR_SLEEP, OUTPUT);
  digitalWrite(MOTOR_SLEEP, HIGH);
  delay(1);

  // use motor reset function to reset internal home
  pinMode(MOTOR_RESET, OUTPUT);
  digitalWrite(MOTOR_RESET, LOW);
  delay(1);
  digitalWrite(MOTOR_RESET, HIGH);
  delay(1);

  // re-enable motor output
  digitalWrite(MOTOR_ENABLE, LOW);
}

// move stepper motor 1 microstep
// takes approx 50 microseconds >> 20,000sps, should use max like 3,000sps
// 0 = cw, 1 = ccw
// TODO: swap this out for conditional code in main loop
void motor_step(uint8_t motor_dir) {
  if (motor_dir == 0) {
    motor_pos_ticks++;
  } else {
    motor_pos_ticks--;
  }
  digitalWrite(MOTOR_DIR, motor_dir);
  delayMicroseconds(3);
  digitalWrite(MOTOR_STEP, HIGH);
  delayMicroseconds(8);
  digitalWrite(MOTOR_STEP, LOW);
  delayMicroseconds(39);
}

// separate the print function
void print_output(void* pvParamters) {
  pendulum_data_t pendulum_data_receive;
  float loop_duration_sec;
  for (;;) {
    // read from queue
    xQueueReceive(pendulum_data_queue, (void*)&pendulum_data_receive, 10);

    // calculations
    loop_duration_sec = pendulum_data_receive.loop_duration * 0.000001;

    printf("ENCODER: ticks: %i | ", pendulum_data_receive.encoder_pos_ticks);
    printf("LOOP: ticks: %i sec: %f Hz: %f",
           pendulum_data_receive.loop_duration, loop_duration_sec, (1 / loop_duration_sec));
    printf("\n");

    // 10ms delay, allow for idle tasks
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// control loop
// TODO: current duration: 2 us
void app_main(void* pvParameters) {
  encoder_pos_ticks = ENCODER_RES / 2;
  motor_init();
  encoder_init();
  vTaskDelay(1 / portTICK_PERIOD_MS);
  int error_steps;

  for (;;) {
    // mark loop start
    loop_start_time = esp_timer_get_time();

    // error calculation in steps
    error_steps = (ENCODER_RES / 2) - encoder_pos_ticks;

    // only fire actuator if within fixable range (45 deg)
    if (error_steps < (ENCODER_RES / 8) && error_steps > SS_ERROR) {
      // TODO error accumulation, actuation, LQR controller
      // base step firing on vel/accel timer
    }

    // send status to print_output thread if queue has room
    pendulum_data_send.encoder_pos_ticks = encoder_pos_ticks;
    pendulum_data_send.motor_pos_ticks = motor_pos_ticks;
    pendulum_data_send.loop_duration = loop_duration;
    xQueueSend(pendulum_data_queue, (void*)&pendulum_data_send, 0);

    // calculate loop time in microseconds
    loop_duration = esp_timer_get_time() - loop_start_time;
  }
}

// global setup
void setup() {
  xTaskCreatePinnedToCore(print_output, "debug_output", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(app_main, "app_main", 4096, NULL, 2, NULL, 1);
}

// required :(
void loop() {
  // yield to other tasks
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
