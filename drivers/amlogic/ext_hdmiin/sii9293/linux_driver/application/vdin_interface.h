#ifndef __VDIN_INTERFACE_H__
#define __VDIN_INTERFACE_H__

int sii5293_register_tvin_frontend(struct tvin_frontend_s *frontend);

void sii9293_stop_tvin(sii9293_tvin_t *info);
int sii9293_start_tvin(sii9293_tvin_t *info, sii_video_timming_link *timming);

#endif
