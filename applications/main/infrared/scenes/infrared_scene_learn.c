#include "../infrared_i.h"
#include <dolphin/dolphin.h>

void infrared_scene_learn_popup_update_text(Popup* popup, InfraredWorker* worker) {
    FuriString* message = furi_string_alloc_printf(
        "Point the remote at IR port\nand push the button.\nDecode: %s",
        infrared_worker_rx_is_signal_decoding_enabled(worker) ? "AUTO" : "OFF");
    popup_set_text(popup, furi_string_get_cstr(message), 5, 15, AlignLeft, AlignCenter);
    furi_string_free(message);
}

bool infrared_scene_learn_popup_input_callback(void* context, InputEvent* event) {
    // toggle decode-mode on OK
    if(event->type == InputTypeShort && event->key == InputKeyOk) {
        Infrared* infrared = context;
        Popup* popup = infrared->popup;
        InfraredWorker* worker = infrared->worker;

        infrared_worker_rx_enable_signal_decoding(
            worker, !infrared_worker_rx_is_signal_decoding_enabled(worker));
        infrared_scene_learn_popup_update_text(popup, worker);

        return true;
    }
    return false;
}

void infrared_scene_learn_on_enter(void* context) {
    Infrared* infrared = context;
    Popup* popup = infrared->popup;
    InfraredWorker* worker = infrared->worker;

    infrared_worker_rx_set_received_signal_callback(
        worker, infrared_signal_received_callback, context);
    infrared_worker_rx_start(worker);
    infrared_play_notification_message(infrared, InfraredNotificationMessageBlinkStartRead);

    popup_set_icon(popup, 0, 32, &I_InfraredLearnShort_128x31);
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignCenter);
    popup_set_context(popup, infrared);
    popup_set_input_callback(popup, infrared_scene_learn_popup_input_callback);
    popup_set_callback(popup, NULL);

    infrared_scene_learn_popup_update_text(popup, worker);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewPopup);
}

bool infrared_scene_learn_on_event(void* context, SceneManagerEvent event) {
    Infrared* infrared = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InfraredCustomEventTypeSignalReceived) {
            infrared_play_notification_message(infrared, InfraredNotificationMessageSuccess);
            scene_manager_next_scene(infrared->scene_manager, InfraredSceneLearnSuccess);
            DOLPHIN_DEED(DolphinDeedIrLearnSuccess);
            consumed = true;
        }
    }

    return consumed;
}

void infrared_scene_learn_on_exit(void* context) {
    Infrared* infrared = context;
    Popup* popup = infrared->popup;
    infrared_worker_rx_set_received_signal_callback(infrared->worker, NULL, NULL);
    infrared_worker_rx_stop(infrared->worker);
    infrared_play_notification_message(infrared, InfraredNotificationMessageBlinkStop);
    popup_set_icon(popup, 0, 0, NULL);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignCenter);
    popup_set_callback(popup, NULL);
    popup_set_input_callback(popup, NULL);
}
