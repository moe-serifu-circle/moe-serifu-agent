// List for using with X-Macro style of maintaining event topics

// Syntax is MSA_EVENT_TOPIC(name, priority)
// Greater priority topics interrupt event handling of lower-numbered
// priority topics.

MSA_EVENT_TOPIC(EVENT_STACK_CLEARED, 0)
MSA_EVENT_TOPIC(EVENT_HANDLED, 0)
MSA_EVENT_TOPIC(EVENT_INTERRUPTED, 0)
MSA_EVENT_TOPIC(TEXT_INPUT, 10)
