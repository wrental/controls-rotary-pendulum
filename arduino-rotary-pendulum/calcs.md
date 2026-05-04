# Motor Settings:

# Encoder:
600P/R >> 1200P/R (with interrupt on both edges)
1 tick = 360 / 1200 = 0.3 deg = 2pi/1200 rad ~= 0.005 rad

# Motor:
- step angle = 1.8 deg 
- 200P/R >> (* 16 microsteps) = 3200P/R
- 1 step = 360 / 3200 = 0.1125 deg = 2pi/3200 rad ~= 0.002 rad
- max speed ~= 3000sps (?) >> 1 step = 1/3000s
  -  1/3200s = 312.5 microseconds
- rated for 1000rpm = 16.67rps
  - 3200P/R * 16 = 51200sps
  - 1 step = 1/51200s = 19.5 microseconds
  - 19.5 microseconds / 3 operations per step = 6.5 microseconds per op
- charts show up to **900RPM** (double stack nema 17) = 15rps
  - 3200P/R * 15 = 48000sps
  - 1 step = 1/48000s = 20.8 microseconds per step
  - 21 microseconds per step / 3 ops per step = 7 microseconds per op
- fixed off time max found in A4988 spec = 40 microseconds
  - assuming 50 microseconds minimum delay (?)

## Motor Testing:
- 10 microseconds/op >> FAIL
- 20 microseconds >> FAIL
- 50 microseconds >> FAIL (misses steps, loses sync)
- **100 microseconds/op >> PASS**
  - dir delay is always 50 microseconds
  - total step time = 50 + 2(step_op_delay)
  - for step_op_delay = 100 >> step time = 250 microseconds = 0.00025s = 1/4000s

# Motor Step:
- 2 microseconds between each op
- 44 microseconds added at end
- 50 microseconds total step time = 0.00005s
- 1s/0.00005s = 20,000sps = 2250 deg/s = 6.25rps = 375rpm

MOTOR DIR = 0 >> CLOCKWISE
