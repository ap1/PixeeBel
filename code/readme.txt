Code navigation information
===========================

Message Processing Code:

TBD

Analytics code:

Most of the code for analytics lies in the file bbb_analytics/bin_recognizer.c. It requires the following:

- bbb_common/bin_reader.c: reads frequency bin specs from a file
- bbb_common/msg_queue.c: helps receive messages from the sensors
- fft/fft4g_h.c: header-only fft library
- bbb_common/pb_visualize.c: helper for sending display commands

The main loop operates on a per-packet basis (256 sampler, 4 channels). It bins the samples in frequency domains and plots the bin values.
The function analyze_msg(...) does most of the magic.

Display code:

TBD