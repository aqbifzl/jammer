void wait_for_start(void){
    xEventGroupClearBits(jammer_event_group, STOPPED_BIT);
    xEventGroupSetBits(jammer_event_group, READY_BIT);
}

void wait_for_stop(void){
    xEventGroupClearBits(jammer_event_group, READY_BIT);

    /* wait for task to ack that it fully stopped (1s timeout here) */
    EventBits_t bits =
        xEventGroupWaitBits(jammer_event_group, STOPPED_BIT,
                            pdTRUE, /* clear STOPPED_BIT when returned */
                            pdTRUE, /* wait for all bits? (we only asked one) */
                            pdMS_TO_TICKS(1000));

    if (!(bits & STOPPED_BIT)) {
      /* timeout — task didn't ack stop in time; handle error */
      printf("warning: jammer didn't stop in time\n");
      /* you can attempt a forceful fallback here, but be careful */
    }
}
