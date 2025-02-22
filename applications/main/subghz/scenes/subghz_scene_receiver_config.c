#include "../subghz_i.h"
#include <lib/toolbox/value_index.h>

#define TAG "SubGhzSceneReceiverConfig"

enum SubGhzSettingIndex {
    SubGhzSettingIndexFrequency,
    SubGhzSettingIndexHopping,
    SubGhzSettingIndexModulation,
    SubGhzSettingIndexBinRAW,
    SubGhzSettingIndexRepeater,
    SubGhzSettingIndexSound,
    SubGhzSettingIndexListenAfterTX,
    SubGhzSettingIndexIgnoreStarline,
    SubGhzSettingIndexIgnoreCars,
    SubGhzSettingIndexIgnoreMagellan,
    SubGhzSettingIndexIgnorePrinceton,
    SubGhzSettingIndexIgnoreNiceFlorS,
    SubGhzSettingIndexResetToDefault,
    SubGhzSettingIndexLock,
    SubGhzSettingIndexRAWThresholdRSSI,
};

#define RAW_THRESHOLD_RSSI_COUNT 11
const char* const raw_threshold_rssi_text[RAW_THRESHOLD_RSSI_COUNT] = {
    "-----",
    "-85.0",
    "-80.0",
    "-75.0",
    "-70.0",
    "-65.0",
    "-60.0",
    "-55.0",
    "-50.0",
    "-45.0",
    "-40.0",

};
const float raw_threshold_rssi_value[RAW_THRESHOLD_RSSI_COUNT] = {
    -90.0f,
    -85.0f,
    -80.0f,
    -75.0f,
    -70.0f,
    -65.0f,
    -60.0f,
    -55.0f,
    -50.0f,
    -45.0f,
    -40.0f,
};

#define COMBO_BOX_COUNT 2
#define REPEATER_BOX_COUNT 4

const uint32_t hopping_value[COMBO_BOX_COUNT] = {
    SubGhzHopperStateOFF,
    SubGhzHopperStateRunning,
};

const uint32_t speaker_value[COMBO_BOX_COUNT] = {
    SubGhzSpeakerStateShutdown,
    SubGhzSpeakerStateEnable,
};

const uint32_t bin_raw_value[COMBO_BOX_COUNT] = {
    SubGhzProtocolFlag_Decodable,
    SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_BinRAW,
};

const uint32_t repeater_value[REPEATER_BOX_COUNT] = {
    SubGhzRepeaterOff,
    SubGhzRepeaterOn,
    SubGhzRepeaterOnLong,
    SubGhzRepeaterOnShort,
};

const uint32_t listen_after_tx_value[COMBO_BOX_COUNT] = {
    false,
    true,
};

const char* const combobox_text[COMBO_BOX_COUNT] = {
    "OFF",
    "ON",
};

const char* const repeater_box_text[REPEATER_BOX_COUNT] = {
    "OFF",
    "Normal",
    "Long",
    "Short",
};

static void
    subghz_scene_receiver_config_set_ignore_filter(VariableItem* item, SubGhzProtocolFlag filter) {
    SubGhz* subghz = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, combobox_text[index]);

    if(index == 0) {
        CLEAR_BIT(subghz->ignore_filter, filter);
    } else {
        SET_BIT(subghz->ignore_filter, filter);
    }

    subghz->last_settings->ignore_filter = subghz->ignore_filter;
}

uint8_t subghz_scene_receiver_config_next_frequency(const uint32_t value, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    SubGhzSetting* setting = subghz_txrx_get_setting(subghz->txrx);

    uint8_t index = 0;
    for(size_t i = 0; i < subghz_setting_get_frequency_count(setting); i++) {
        if(value == subghz_setting_get_frequency(setting, i)) {
            index = i;
            break;
        } else {
            index = subghz_setting_get_frequency_default_index(setting);
        }
    }
    return index;
}

uint8_t subghz_scene_receiver_config_next_preset(const char* preset_name, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    uint8_t index = 0;
    SubGhzSetting* setting = subghz_txrx_get_setting(subghz->txrx);

    for(size_t i = 0; i < subghz_setting_get_preset_count(setting); i++) {
        if(!strcmp(subghz_setting_get_preset_name(setting, i), preset_name)) {
            index = i;
            break;
        } else {
            //  index = subghz_setting_get_frequency_default_index(setting);
        }
    }
    return index;
}

SubGhzHopperState subghz_scene_receiver_config_hopper_value_index(void* context) {
    furi_assert(context);
    SubGhz* subghz = context;

    if(subghz_txrx_hopper_get_state(subghz->txrx) == SubGhzHopperStateOFF) {
        return SubGhzHopperStateOFF;
    } else {
        variable_item_set_current_value_text(
            (VariableItem*)scene_manager_get_scene_state(
                subghz->scene_manager, SubGhzSceneReceiverConfig),
            " -----");
        return SubGhzHopperStateRunning;
    }
}

static void subghz_scene_receiver_config_set_frequency(VariableItem* item) {
    SubGhz* subghz = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    SubGhzSetting* setting = subghz_txrx_get_setting(subghz->txrx);

    if(subghz_txrx_hopper_get_state(subghz->txrx) == SubGhzHopperStateOFF) {
        char text_buf[10] = {0};
        uint32_t frequency = subghz_setting_get_frequency(setting, index);
        SubGhzRadioPreset preset = subghz_txrx_get_preset(subghz->txrx);

        snprintf(
            text_buf,
            sizeof(text_buf),
            "%lu.%02lu",
            frequency / 1000000,
            (frequency % 1000000) / 10000);
        variable_item_set_current_value_text(item, text_buf);
        subghz_txrx_set_preset(
            subghz->txrx,
            furi_string_get_cstr(preset.name),
            frequency,
            preset.data,
            preset.data_size);

        preset = subghz_txrx_get_preset(subghz->txrx);

        subghz->last_settings->frequency = preset.frequency;
        subghz_setting_set_default_frequency(setting, preset.frequency);
    } else {
        variable_item_set_current_value_index(
            item, subghz_setting_get_frequency_default_index(setting));
    }
}

static void subghz_scene_receiver_config_set_preset(VariableItem* item) {
    SubGhz* subghz = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    SubGhzSetting* setting = subghz_txrx_get_setting(subghz->txrx);

    const char* preset_name = subghz_setting_get_preset_name(setting, index);
    variable_item_set_current_value_text(item, preset_name);
    //subghz->last_settings->preset = index;
    SubGhzRadioPreset preset = subghz_txrx_get_preset(subghz->txrx);
    subghz_txrx_set_preset(
        subghz->txrx,
        preset_name,
        preset.frequency,
        subghz_setting_get_preset_data(setting, index),
        subghz_setting_get_preset_data_size(setting, index));
    subghz->last_settings->preset_index = index;
}

static void subghz_scene_receiver_config_set_hopping_running(VariableItem* item) {
    SubGhz* subghz = variable_item_get_context(item);
    SubGhzHopperState index = variable_item_get_current_value_index(item);
    SubGhzSetting* setting = subghz_txrx_get_setting(subghz->txrx);
    VariableItem* frequency_item = (VariableItem*)scene_manager_get_scene_state(
        subghz->scene_manager, SubGhzSceneReceiverConfig);

    variable_item_set_current_value_text(item, combobox_text[(uint8_t)index]);

    if(index == SubGhzHopperStateOFF) {
        char text_buf[10] = {0};
        uint32_t frequency = subghz_setting_get_default_frequency(setting);
        SubGhzRadioPreset preset = subghz_txrx_get_preset(subghz->txrx);

        snprintf(
            text_buf,
            sizeof(text_buf),
            "%lu.%02lu",
            frequency / 1000000,
            (frequency % 1000000) / 10000);
        variable_item_set_current_value_text(frequency_item, text_buf);

        // Maybe better add one more function with only with the frequency argument?
        subghz_txrx_set_preset(
            subghz->txrx,
            furi_string_get_cstr(preset.name),
            frequency,
            preset.data,
            preset.data_size);
        variable_item_set_current_value_index(
            frequency_item, subghz_setting_get_frequency_default_index(setting));
    } else {
        variable_item_set_current_value_text(frequency_item, " -----");
        variable_item_set_current_value_index(
            frequency_item, subghz_setting_get_frequency_default_index(setting));
    }
    subghz->last_settings->enable_hopping = index != SubGhzHopperStateOFF;
    subghz_txrx_hopper_set_state(subghz->txrx, index);
}

static void subghz_scene_receiver_config_set_speaker(VariableItem* item) {
    SubGhz* subghz = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, combobox_text[index]);
    subghz_txrx_speaker_set_state(subghz->txrx, speaker_value[index]);
    subghz->last_settings->enable_sound = (speaker_value[index] == SubGhzSpeakerStateEnable);
}

static void subghz_scene_receiver_config_set_bin_raw(VariableItem* item) {
    SubGhz* subghz = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, combobox_text[index]);
    subghz->filter = bin_raw_value[index];
    subghz_txrx_receiver_set_filter(subghz->txrx, subghz->filter);

    // We can set here, but during subghz_last_settings_save filter was changed to ignore BinRAW
    subghz->last_settings->filter = subghz->filter;
    subghz->BINRawStateChanged = false;
}

static void subghz_scene_receiver_config_set_listen_after_tx(VariableItem* item) {
    SubGhz* subghz = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, combobox_text[index]);
    subghz->ListenAfterTX = listen_after_tx_value[index];

    // We can set here, but during subghz_last_settings_save filter was changed to ignore BinRAW
    subghz->last_settings->enable_listen_after_tx = subghz->ListenAfterTX;
}

static void subghz_scene_receiver_config_set_repeater(VariableItem* item) {
    SubGhz* subghz = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, repeater_box_text[index]);
    subghz_txrx_repeater_set_state(&subghz->repeater, repeater_value[index]);

    //Change BinRAW to ON or OFF as required, and remember whether I changed it! (Put back for the user.)
    if(repeater_value[index] != SubGhzRepeaterOff) {
        subghz->last_settings->RepeaterState = repeater_value[index];

        if((bin_raw_value[variable_item_get_current_value_index(subghz->BIN_Raw_menu)] &
            SubGhzProtocolFlag_BinRAW) == SubGhzProtocolFlag_BinRAW) {
            //BinRAW is on, repeater is on.
        } else {
            //Repeater is on, Binraw is Off.
            variable_item_set_current_value_index(
                subghz->BIN_Raw_menu, 1 /*Index of ON in BIN_Raw menu!*/);
            subghz_scene_receiver_config_set_bin_raw(subghz->BIN_Raw_menu);
            subghz->BINRawStateChanged = true;
        }

        //Lock the BinRAW menu, Flipper doesnt understand everything so BinRAW makes very key send.
        variable_item_set_locked(
            subghz->BIN_Raw_menu, true, "Turn off\n Repeater!\n to do\n that!");
        /* REPEATER LOCK BINRAW */
    } else {
        subghz->last_settings->RepeaterState = SubGhzRepeaterOff;

        //Put BinRAW back how it was, if we changed it.
        if(subghz->BINRawStateChanged) {
            variable_item_set_current_value_index(
                subghz->BIN_Raw_menu, 0 /*Index of OFF in BIN_Raw menu!*/);
            subghz_scene_receiver_config_set_bin_raw(subghz->BIN_Raw_menu);
            subghz->BINRawStateChanged = false;
        }

        //Lock the BinRAW menu, Flipper doesnt understand everything so BinRAW makes very key send.
        variable_item_set_locked(
            subghz->BIN_Raw_menu, false, "Turn off\n Repeater!\n to do\n that!");
        /* REPEATER LOCK BINRAW */
    }
}

static void subghz_scene_receiver_config_set_raw_threshold_rssi(VariableItem* item) {
    SubGhz* subghz = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, raw_threshold_rssi_text[index]);
    subghz_threshold_rssi_set(subghz->threshold_rssi, raw_threshold_rssi_value[index]);

    subghz->last_settings->rssi = raw_threshold_rssi_value[index];
}

static inline bool subghz_scene_receiver_config_ignore_filter_get_index(
    SubGhzProtocolFlag filter,
    SubGhzProtocolFlag flag) {
    return READ_BIT(filter, flag) > 0;
}

static void subghz_scene_receiver_config_set_starline(VariableItem* item) {
    subghz_scene_receiver_config_set_ignore_filter(item, SubGhzProtocolFlag_StarLine);
}

static void subghz_scene_receiver_config_set_auto_alarms(VariableItem* item) {
    subghz_scene_receiver_config_set_ignore_filter(item, SubGhzProtocolFlag_AutoAlarms);
}

static void subghz_scene_receiver_config_set_magellan(VariableItem* item) {
    subghz_scene_receiver_config_set_ignore_filter(item, SubGhzProtocolFlag_Magellan);
}

static void subghz_scene_receiver_config_set_princeton(VariableItem* item) {
    subghz_scene_receiver_config_set_ignore_filter(item, SubGhzProtocolFlag_Princeton);
}

static void subghz_scene_receiver_config_set_niceflors(VariableItem* item) {
    subghz_scene_receiver_config_set_ignore_filter(item, SubGhzProtocolFlag_NiceFlorS);
}

static void subghz_scene_receiver_config_var_list_enter_callback(void* context, uint32_t index) {
    furi_assert(context);
    SubGhz* subghz = context;
    if(index == SubGhzSettingIndexLock) {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubGhzCustomEventSceneSettingLock);
    } else if(index == SubGhzSettingIndexResetToDefault) {
        // Reset all values to default state!
#if SUBGHZ_LAST_SETTING_SAVE_PRESET
        subghz_txrx_set_preset_internal(
            subghz->txrx,
            SUBGHZ_LAST_SETTING_DEFAULT_FREQUENCY,
            SUBGHZ_LAST_SETTING_DEFAULT_PRESET);
#else
        subghz_txrx_set_default_preset(subghz->txrx, SUBGHZ_LAST_SETTING_DEFAULT_FREQUENCY);
#endif
        SubGhzSetting* setting = subghz_txrx_get_setting(subghz->txrx);
        SubGhzRadioPreset preset = subghz_txrx_get_preset(subghz->txrx);
        const char* preset_name = furi_string_get_cstr(preset.name);
        int preset_index = subghz_setting_get_inx_preset_by_name(setting, preset_name);
        const int default_index = 0;

        subghz->last_settings->frequency = preset.frequency;
        subghz->last_settings->preset_index = preset_index;
        subghz->last_settings->RepeaterState = SubGhzRepeaterOff;
        subghz_txrx_repeater_set_state(&subghz->repeater, SubGhzRepeaterOff);
        subghz->BINRawStateChanged = false;
        subghz_threshold_rssi_set(subghz->threshold_rssi, raw_threshold_rssi_value[default_index]);
        subghz->filter = bin_raw_value[0];
        subghz->ignore_filter = 0x00;
        subghz_txrx_receiver_set_filter(subghz->txrx, subghz->filter);
        subghz->last_settings->ignore_filter = subghz->ignore_filter;
        subghz->last_settings->filter = subghz->filter;

        subghz_txrx_speaker_set_state(subghz->txrx, speaker_value[default_index]);

        subghz_txrx_hopper_set_state(subghz->txrx, hopping_value[default_index]);
        subghz->last_settings->enable_hopping = hopping_value[default_index];

        variable_item_list_set_selected_item(subghz->variable_item_list, default_index);
        variable_item_list_reset(subghz->variable_item_list);
#ifdef FURI_DEBUG
        subghz_last_settings_log(subghz->last_settings);
#endif
        subghz_last_settings_save(subghz->last_settings);

        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubGhzCustomEventSceneSettingResetToDefault);
    }
}

void subghz_scene_receiver_config_on_enter(void* context) {
    SubGhz* subghz = context;
    VariableItem* item;
    uint8_t value_index;
    SubGhzSetting* setting = subghz_txrx_get_setting(subghz->txrx);
    SubGhzRadioPreset preset = subghz_txrx_get_preset(subghz->txrx);

    //This gets checked a few times, so lets cache this.
    bool raw_send_only =
        (scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneReadRAW) ==
         SubGhzCustomEventManagerSet);
    //bool ListenAfterTX = subghz->last_settings->enable_listen_after_tx;

    item = variable_item_list_add(
        subghz->variable_item_list,
        "Frequency",
        subghz_setting_get_frequency_count(setting),
        subghz_scene_receiver_config_set_frequency,
        subghz);
    value_index = subghz_scene_receiver_config_next_frequency(preset.frequency, subghz);
    scene_manager_set_scene_state(
        subghz->scene_manager, SubGhzSceneReceiverConfig, (uint32_t)item);
    variable_item_set_current_value_index(item, value_index);
    char text_buf[10] = {0};
    uint32_t frequency = subghz_setting_get_frequency(setting, value_index);
    snprintf(
        text_buf,
        sizeof(text_buf),
        "%lu.%02lu",
        frequency / 1000000,
        (frequency % 1000000) / 10000);
    variable_item_set_current_value_text(item, text_buf);

    item = variable_item_list_add(
        subghz->variable_item_list,
        "Modulation",
        subghz_setting_get_preset_count(setting),
        subghz_scene_receiver_config_set_preset,
        subghz);
    value_index =
        subghz_scene_receiver_config_next_preset(furi_string_get_cstr(preset.name), subghz);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(
        item, subghz_setting_get_preset_name(setting, value_index));

    if(!raw_send_only) {
        // Hopping
        item = variable_item_list_add(
            subghz->variable_item_list,
            "Hopping",
            COMBO_BOX_COUNT,
            subghz_scene_receiver_config_set_hopping_running,
            subghz);
        value_index = subghz_scene_receiver_config_hopper_value_index(subghz);

        variable_item_set_current_value_index(item, value_index);
        variable_item_set_current_value_text(item, combobox_text[value_index]);
    }

    if(!raw_send_only) {
        item = variable_item_list_add(
            subghz->variable_item_list,
            "RAW Receive",
            COMBO_BOX_COUNT,
            subghz_scene_receiver_config_set_bin_raw,
            subghz);

        value_index = value_index_uint32(subghz->filter, bin_raw_value, COMBO_BOX_COUNT);
        variable_item_set_current_value_index(item, value_index);
        variable_item_set_current_value_text(item, combobox_text[value_index]);
        subghz->BIN_Raw_menu = item;
    }

    /* Repeater Menu. With the Repeater, you can relay a key. 
       To Use, select the frequency (or hopper) and enable Repeater.
       Any keys decoded with BinRAW will be transmitted as soon as they are received.
       Ill leave the uses of this one up to your imagination! */
    if(!raw_send_only) {
        item = variable_item_list_add(
            subghz->variable_item_list,
            "Repeater",
            REPEATER_BOX_COUNT,
            subghz_scene_receiver_config_set_repeater,
            subghz);
        value_index = value_index_uint32(
            subghz_txrx_repeater_get_state(&subghz->repeater), repeater_value, REPEATER_BOX_COUNT);
        variable_item_set_current_value_index(item, value_index);
        //variable_item_set_current_value_text(item, repeater_box_text[value_index]);
        //Instead of setting the text, Im calling the callback. That sets the text, and the bin raw on off!
        subghz_scene_receiver_config_set_repeater(item);
        //Remember if the user set BinRAW on or if I did with the repeater!
        subghz->BINRawStateChanged =
            (subghz->last_settings->filter !=
             (SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_BinRAW));
    }

    // Enable speaker, will send all incoming noises and signals to speaker so you can listen how your remote sounds like :)
    item = variable_item_list_add(
        subghz->variable_item_list,
        "Sound",
        COMBO_BOX_COUNT,
        subghz_scene_receiver_config_set_speaker,
        subghz);
    value_index = value_index_uint32(
        subghz_txrx_speaker_get_state(subghz->txrx), speaker_value, COMBO_BOX_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, combobox_text[value_index]);

    /* And now we have the WakeUp commmand for the Repeater. Set this, record the wake up code from the car.
        take wake up command to fob, play back and be in repeater or read to get the key to open and start.
        Power of Repeated command may have to be played with in future so cars dont detect too much power from key.
        Im sure theres a milion other ways you can set this up to do other stuff, but the Wake Up is easy now!
        Time to imagine the other uses, and make sure we can accomdateg them! 
    */
    //if(!subghz->raw_send_only) { //Cant switch to receive if the app is opened TX only! (We never get here either, but just to be safe!)
    item = variable_item_list_add(
        subghz->variable_item_list,
        "Listen after TX",
        COMBO_BOX_COUNT,
        subghz_scene_receiver_config_set_listen_after_tx,
        subghz);
    value_index =
        value_index_uint32(subghz->ListenAfterTX, listen_after_tx_value, COMBO_BOX_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, combobox_text[value_index]);
    //}

    if(!raw_send_only) {
        item = variable_item_list_add(
            subghz->variable_item_list,
            "Ignore Starline",
            COMBO_BOX_COUNT,
            subghz_scene_receiver_config_set_starline,
            subghz);

        value_index = subghz_scene_receiver_config_ignore_filter_get_index(
            subghz->ignore_filter, SubGhzProtocolFlag_StarLine);
        variable_item_set_current_value_index(item, value_index);
        variable_item_set_current_value_text(item, combobox_text[value_index]);

        item = variable_item_list_add(
            subghz->variable_item_list,
            "Ignore Cars",
            COMBO_BOX_COUNT,
            subghz_scene_receiver_config_set_auto_alarms,
            subghz);

        value_index = subghz_scene_receiver_config_ignore_filter_get_index(
            subghz->ignore_filter, SubGhzProtocolFlag_AutoAlarms);
        variable_item_set_current_value_index(item, value_index);
        variable_item_set_current_value_text(item, combobox_text[value_index]);

        item = variable_item_list_add(
            subghz->variable_item_list,
            "Ignore Magellan",
            COMBO_BOX_COUNT,
            subghz_scene_receiver_config_set_magellan,
            subghz);

        value_index = subghz_scene_receiver_config_ignore_filter_get_index(
            subghz->ignore_filter, SubGhzProtocolFlag_Magellan);
        variable_item_set_current_value_index(item, value_index);
        variable_item_set_current_value_text(item, combobox_text[value_index]);

        item = variable_item_list_add(
            subghz->variable_item_list,
            "Ignore Princeton",
            COMBO_BOX_COUNT,
            subghz_scene_receiver_config_set_princeton,
            subghz);

        value_index = subghz_scene_receiver_config_ignore_filter_get_index(
            subghz->ignore_filter, SubGhzProtocolFlag_Princeton);
        variable_item_set_current_value_index(item, value_index);
        variable_item_set_current_value_text(item, combobox_text[value_index]);

        item = variable_item_list_add(
            subghz->variable_item_list,
            "Ignore Nice Flor-S / Nice One",
            COMBO_BOX_COUNT,
            subghz_scene_receiver_config_set_niceflors,
            subghz);

        value_index = subghz_scene_receiver_config_ignore_filter_get_index(
            subghz->ignore_filter, SubGhzProtocolFlag_NiceFlorS);
        variable_item_set_current_value_index(item, value_index);
        variable_item_set_current_value_text(item, combobox_text[value_index]);
    }

    if(!raw_send_only) {
        // Reset to default
        variable_item_list_add(subghz->variable_item_list, "Reset to default", 1, NULL, NULL);

        variable_item_list_set_enter_callback(
            subghz->variable_item_list,
            subghz_scene_receiver_config_var_list_enter_callback,
            subghz);
    }
    if(!raw_send_only) {
        // Lock keyboard
        variable_item_list_add(subghz->variable_item_list, "Lock Keyboard", 1, NULL, NULL);
        variable_item_list_set_enter_callback(
            subghz->variable_item_list,
            subghz_scene_receiver_config_var_list_enter_callback,
            subghz);
    }

    if(raw_send_only) {
        item = variable_item_list_add(
            subghz->variable_item_list,
            "RSSI Threshold:",
            RAW_THRESHOLD_RSSI_COUNT,
            subghz_scene_receiver_config_set_raw_threshold_rssi,
            subghz);
        value_index = value_index_float(
            subghz_threshold_rssi_get(subghz->threshold_rssi),
            raw_threshold_rssi_value,
            RAW_THRESHOLD_RSSI_COUNT);
        variable_item_set_current_value_index(item, value_index);
        variable_item_set_current_value_text(item, raw_threshold_rssi_text[value_index]);
    }
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdVariableItemList);
}

bool subghz_scene_receiver_config_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventSceneSettingLock) {
            subghz_lock(subghz);
            scene_manager_previous_scene(subghz->scene_manager);
            consumed = true;
        } else if(event.event == SubGhzCustomEventSceneSettingResetToDefault) {
            scene_manager_previous_scene(subghz->scene_manager);
            consumed = true;
        }
    }
    return consumed;
}

void subghz_scene_receiver_config_on_exit(void* context) {
    SubGhz* subghz = context;

    //If the Repeater changed the BinRAW settings, put them back now. That will ensure it loads correctly!
    if(subghz->BINRawStateChanged & (subghz->repeater != SubGhzRepeaterOff)) {
        //Bit hacky, but works well.
        subghz->last_settings->filter = bin_raw_value[0 /*BinRAW Off*/];
        subghz->BINRawStateChanged = false;
    }
    subghz->BIN_Raw_menu = NULL;

    variable_item_list_set_selected_item(subghz->variable_item_list, 0);
    variable_item_list_reset(subghz->variable_item_list);
#ifdef FURI_DEBUG
    subghz_last_settings_log(subghz->last_settings);
#endif
    subghz_last_settings_save(subghz->last_settings);
    scene_manager_set_scene_state(
        subghz->scene_manager, SubGhzSceneReadRAW, SubGhzCustomEventManagerNoSet);
}
