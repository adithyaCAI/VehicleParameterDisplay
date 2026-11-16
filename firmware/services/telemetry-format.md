# VPD1 serial record

Transport: ST-Link VCP USART2, 115200 baud, 8N1. This is the firmware's selected local dashboard transport; no external dashboard application has been supplied or validated.

Accept only complete CRLF-delimited lines starting `VPD1,`; other lines are diagnostics. Maximum line storage is 256 bytes. Default period is 200 ms; cooperative scheduling can introduce jitter, missed slots are not replayed. UART sends at most one ready byte per main-loop iteration, from a single copied record. Busy queue drops the next record; cumulative drops are reported.

Fields, in order:

1. VPD1: protocol version
2. sequence: unsigned 32-bit record sequence, wrapping
3. uptime_ms: unsigned 32-bit milliseconds, wrapping
4. climate_valid: 0 or 1
5. temperature_mC: signed thousandths of degrees Celsius
6. humidity_millipercent: signed thousandths of percent RH
7. acceleration_valid: 0 or 1
8. lateral_x_mg: signed thousandths of g
9. longitudinal_y_mg: signed thousandths of g
10. vertical_z_mg: signed thousandths of g
11. event: 0 none, 1 Speeding, 2 Wide Turn, 3 Unseen SpeedBreaker, 4 Sudden Braking
12. dropped: unsigned cumulative queue/format drops, wrapping

Invalid numerical fields use -2147483648 and validity 0; never interpret them as measurements. Conversion truncates toward zero. Example valid record:

`VPD1,7,1600,1,23700,54400,1,-2,2,1000,0,0`

This represents 23.7 C, 54.4 percent RH, X=-0.002 g, Y=0.002 g, Z=1 g and no event.

Diagnostic messages are dropped while telemetry owns the UART to avoid inserting bytes inside a frame. No delivery acknowledgment, retransmission, persistent storage, remote server API or integrity checksum is provided. Physical dashboard ingestion remains an acceptance check.
