# Semaphores
This program is an implementation of producer-consumer scenario.

There are three consumers, one producent and a FIFO buffer.
Producer puts new elements into the buffer.
Elements are deleted from the buffer arter being read by both A and B, or immiedietly after being read by C.
When an element has already been read by A or B it should no longer be readable by C.
