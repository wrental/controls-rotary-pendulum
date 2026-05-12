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

// not sure these are really needed
#define SS_ERROR 3                          // steady-state error margin (ticks)
#define ACTUATION_RANGE (ENCODER_RES / 10)  // 360 / 10 = 36 deg
#define MOTOR_VEL_MAX 1000                  // sps
#define MOTOR_ACCEL_MAX 3000                // spsps
#define STEP_ACTION_DELAY 2                 // microseconds
#define LOOP_DURATION 1                     // ms = 1kHz

// PID constants
#define K_P 0.1
#define K_I 1.9
#define K_D 8.8

// data sharing between threads
typedef struct pendulum_data_t {
  float encoder_pos_degrees;
  float move_stp_degrees;
};

pendulum_data_t pendulum_data_send;
int queue_item_size = sizeof(pendulum_data_send);
QueueHandle_t pendulum_data_queue = xQueueCreate(2, queue_item_size);

// encoder:
int encoder_pos_ticks = 0;  // cw = positive, ccw = negative

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
// takes approx 50 microseconds >> 20,000sps, should use max like 3,000spsps
// 0 = cw, 1 = ccw
// TODO: swap this out for conditional code in main loop
void motor_step(uint8_t motor_dir) {
  // if (motor_dir == 0) {
  //   motor_pos_ticks++;
  // } else {
  //   motor_pos_ticks--;
  // }
  digitalWrite(MOTOR_DIR, motor_dir);
  delayMicroseconds(3);
  digitalWrite(MOTOR_STEP, HIGH);
  delayMicroseconds(8);
  digitalWrite(MOTOR_STEP, LOW);
  delayMicroseconds(39);
}

void motor_stp_degrees(float degrees) {
  int motor_dir;
  int steps = (int)(degrees * (3200.0 / 360.0));
  if (steps < 0) {
    motor_dir = 0;
  } else {
    motor_dir = 1;
  }
  for (int i = 0; i < steps; i++) {
    motor_step(motor_dir);
  }
}

// separate the print function - CORE 0
void print_output(void* pvParamters) {
  pendulum_data_t pendulum_data_receive;
  float loop_duration_sec;
  for (;;) {
    // read from queue
    xQueueReceive(pendulum_data_queue, (void*)&pendulum_data_receive, 10);

    printf("encoder_pos_degrees %.2f | move_stp_degrees %.2f",
           pendulum_data_receive.encoder_pos_degrees, pendulum_data_receive.move_stp_degrees);
    printf("\n");

    // 10ms delay, allow for idle tasks
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// control loop - CORE 1
// TODO: current duration: 2 us
void app_main(void* pvParameters) {
  // motor switching variables
  uint8_t motor_dir = 0;  // 0 = cw, 1 = ccw
  int motor_pos_ticks = 0;
  int last_step_action_time = 0;
  bool motor_step_status = false;

  // encoder position initialization
  encoder_pos_ticks = 0;  // starting position at bottom - DO NOT REINIT GLOBAL VAR
  int error_steps = 0;

  // init stepper, encoder, ISR
  motor_init();
  encoder_init();

  // momentary delay
  vTaskDelay(1 / portTICK_PERIOD_MS);

  // 1kHz implementation
  // TickType_t loop_start_ticks;

  // asap timer implementation
  int t = 0;
  int dt = 0;
  float dt_s;

  int loop_enc_pos_ticks;
  float encoder_pos_degrees;
  float encoder_err_degrees;
  float last_err_degrees;
  float integral;
  float derivative;
  float move_stp_degrees;

  // main app loop
  for (;;) {
    // 1kHz implementation
    // vTaskDelayUntil(&loop_start_ticks, pdMS_TO_TICKS(LOOP_DURATION));

    dt = esp_timer_get_time() - t;
    dt_s = dt / 1000000.0;
    t = esp_timer_get_time();
    last_err_degrees = encoder_err_degrees;

    // bottom = 0, top = 180 deg
    loop_enc_pos_ticks = abs(encoder_pos_ticks % 2400);
    encoder_pos_degrees = (loop_enc_pos_ticks * (360.0 / 2400.0));
    encoder_err_degrees = 180.0 - encoder_pos_degrees;
    integral = encoder_err_degrees * dt_s;
    derivative = last_err_degrees - encoder_err_degrees;
    move_stp_degrees = (K_P * encoder_err_degrees) + (K_I * integral) + (K_D * derivative);

    // only actuate if within 20 deg error
    if (abs(encoder_err_degrees) < 20) {
      motor_stp_degrees(move_stp_degrees);
    }

    // send status to print_output thread if queue has room
    pendulum_data_send.encoder_pos_degrees = encoder_pos_degrees;
    pendulum_data_send.move_stp_degrees = move_stp_degrees;
    xQueueSend(pendulum_data_queue, (void*)&pendulum_data_send, 0);
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
