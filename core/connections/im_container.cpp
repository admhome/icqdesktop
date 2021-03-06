#include "stdafx.h"
#include "im_container.h"
#include "login_info.h"
#include "im_login.h"
#include "../../corelib/collection_helper.h"
#include "../../corelib/enumerations.h"
#include "wim/wim_im.h"
#include "wim/wim_packet.h"
#include "../Voip/VoipManager.h"

#include "../core.h"
#include "../utils.h"
#include "../archive/contact_archive.h"
#include "search_contacts_params.h"
#include "../statistics.h"
#include "../themes/themes.h"
#include "../proxy_settings.h"

#include "../configuration/app_config.h"

using namespace core;

im_container::im_container(voip_manager::VoipManager& voip_manager)
    :   voip_manager_(voip_manager),
        logins_(new im_login_list(utils::get_product_data_path() + L"/settings/ims.stg"))
{
    REGISTER_IM_MESSAGE("login_by_password", on_login_by_password);
    REGISTER_IM_MESSAGE("login_by_password_for_attach_uin", on_login_by_password_for_attach_uin);
    REGISTER_IM_MESSAGE("login_get_sms_code", on_login_get_sms_code);
    REGISTER_IM_MESSAGE("login_by_phone", on_login_by_phone);
    REGISTER_IM_MESSAGE("logout", on_logout);
    REGISTER_IM_MESSAGE("connect_after_migration", on_connect_after_migration);
    REGISTER_IM_MESSAGE("avatars/get", on_get_contact_avatar);
    REGISTER_IM_MESSAGE("avatars/show", on_show_contact_avatar);
    REGISTER_IM_MESSAGE("send_message", on_send_message);
    REGISTER_IM_MESSAGE("message/typing", on_message_typing);
    REGISTER_IM_MESSAGE("feedback/send", on_feedback);
    REGISTER_IM_MESSAGE("set_state", on_set_state);
    REGISTER_IM_MESSAGE("archive/index/get", on_get_archive_index);
    REGISTER_IM_MESSAGE("archive/buddies/get", on_get_archive_messages_buddies);
    REGISTER_IM_MESSAGE("archive/messages/get", on_get_archive_messages);
    REGISTER_IM_MESSAGE("archive/messages/delete", on_delete_archive_messages);
    REGISTER_IM_MESSAGE("archive/messages/delete_from", on_delete_archive_messages_from);
    REGISTER_IM_MESSAGE("search", on_search);
    REGISTER_IM_MESSAGE("dialogs/add", on_add_opened_dialog);
    REGISTER_IM_MESSAGE("dialogs/remove", on_remove_opened_dialog);
    REGISTER_IM_MESSAGE("dialogs/set_first_message", on_set_first_message);
    REGISTER_IM_MESSAGE("dialogs/hide", on_hide_chat);
    REGISTER_IM_MESSAGE("dialogs/mute", on_mute_chat);
    REGISTER_IM_MESSAGE("dlg_state/set_last_read", on_set_last_read);
    REGISTER_IM_MESSAGE("voip_call", on_voip_call_message);
    REGISTER_IM_MESSAGE("files/upload", on_upload_file_sharing);
    REGISTER_IM_MESSAGE("files/upload/abort", on_abort_file_sharing_uploading);
    REGISTER_IM_MESSAGE("files/download", on_download_file);
    REGISTER_IM_MESSAGE("files/download/abort", on_abort_file_downloading);
    REGISTER_IM_MESSAGE("image/download", on_download_image);
    REGISTER_IM_MESSAGE("stickers/meta/get", on_get_stickers_meta);
    REGISTER_IM_MESSAGE("stickers/sticker/get", on_get_sticker);
    REGISTER_IM_MESSAGE("chats/info/get", on_get_chat_info);
    REGISTER_IM_MESSAGE("chats/blocked/get", on_get_chat_blocked);
    REGISTER_IM_MESSAGE("chats/home/get", on_get_chat_home);
    REGISTER_IM_MESSAGE("contacts/search", on_search_contacts);
    REGISTER_IM_MESSAGE("contacts/search2", on_search_contacts2);
    REGISTER_IM_MESSAGE("contacts/profile/get", on_profile);
    REGISTER_IM_MESSAGE("contacts/add", on_add_contact);
    REGISTER_IM_MESSAGE("contacts/remove", on_remove_contact);
    REGISTER_IM_MESSAGE("contacts/rename", on_rename_contact);
    REGISTER_IM_MESSAGE("contacts/block", on_spam_contact);
    REGISTER_IM_MESSAGE("contacts/ignore", on_ignore_contact);
    REGISTER_IM_MESSAGE("contacts/get_ignore", on_get_ignore_contacts);
    REGISTER_IM_MESSAGE("dlg_state/hide", on_hide_dlg_state);
    REGISTER_IM_MESSAGE("remove_members", on_remove_members);
    REGISTER_IM_MESSAGE("add_members", on_add_members);
    REGISTER_IM_MESSAGE("add_chat", on_add_chat);
    REGISTER_IM_MESSAGE("modify_chat", on_modify_chat);
    REGISTER_IM_MESSAGE("sign_url", on_sign_url);
    REGISTER_IM_MESSAGE("stats", on_stats);
    REGISTER_IM_MESSAGE("themes/meta/get", on_get_themes_meta);
    REGISTER_IM_MESSAGE("themes/theme/get", on_get_theme);
    REGISTER_IM_MESSAGE("files/set_url_played", on_url_played);
    REGISTER_IM_MESSAGE("files/speech_to_text", on_speech_to_text);
    REGISTER_IM_MESSAGE("favorite", on_favorite);
    REGISTER_IM_MESSAGE("unfavorite", on_unfavorite);
    REGISTER_IM_MESSAGE("load_flags", on_get_flags);
    REGISTER_IM_MESSAGE("update_profile", on_update_profile);
    REGISTER_IM_MESSAGE("set_user_proxy_settings", on_set_user_proxy);
    REGISTER_IM_MESSAGE("livechat/join", on_join_livechat);
    REGISTER_IM_MESSAGE("set_locale", on_set_locale);
    REGISTER_IM_MESSAGE("set_avatar", on_set_avatar);
    REGISTER_IM_MESSAGE("chats/mod/name", on_mod_chat_name);
    REGISTER_IM_MESSAGE("chats/mod/about", on_mod_chat_about);
    REGISTER_IM_MESSAGE("chats/mod/public", on_mod_chat_public);
    REGISTER_IM_MESSAGE("chats/block", on_block_chat_member);
    REGISTER_IM_MESSAGE("chats/role/set", on_set_chat_member_role);
    REGISTER_IM_MESSAGE("close_promo", on_close_promo);
    REGISTER_IM_MESSAGE("phoneinfo", on_phoneinfo);
}


im_container::~im_container()
{
}

void core::im_container::create()
{
    if (create_ims())
    {
        connect_ims();
    }
}

void im_container::connect_ims()
{
    for (auto iter = ims_.begin(); iter < ims_.end(); iter++)
        (*iter)->connect();
}

bool im_container::update_login(im_login_id& _login)
{
    return logins_->update(_login);
}

void im_container::replace_uin_in_login(im_login_id& old_login, im_login_id& new_login)
{
    logins_->replace_uin(old_login, new_login);
}

bool core::im_container::create_ims()
{
    if (logins_->load())
    {
    }

    im_login_id login("", default_im_id);

    if (logins_->get_first_login(login))
    {
    }

    ims_.push_back(std::make_shared<wim::im>(login, voip_manager_));

    return !ims_.empty();
}

std::string core::im_container::get_first_login() const
{
    im_login_id login("", default_im_id);
    logins_->get_first_login(login);
    return login.get_login();
}

void core::im_container::on_message_from_gui(const char * _message, int64_t _seq, coll_helper& _params)
{
    auto iter_handler = messages_map_.find(_message);
    if (iter_handler == messages_map_.end())
    {
        assert(!"unknown message type");
        return;
    }

    iter_handler->second(_seq, _params);
}

void core::im_container::fromInternalProxySettings2Voip(const core::proxy_settings& proxySettings, voip_manager::VoipProxySettings& voipProxySettings) {
    using namespace voip_manager;

    if (!proxySettings.use_proxy_) {
        voipProxySettings.type          = VoipProxySettings::kProxyType_None;
    } else {
        switch (proxySettings.proxy_type_) {
        case 0:  voipProxySettings.type = VoipProxySettings::kProxyType_Http;
        case 4:  voipProxySettings.type = VoipProxySettings::kProxyType_Socks4;
        case 5:  voipProxySettings.type = VoipProxySettings::kProxyType_Socks5;
        case 6:  voipProxySettings.type = VoipProxySettings::kProxyType_Socks4a;
        default: voipProxySettings.type = VoipProxySettings::kProxyType_None;
        }
    }

    voipProxySettings.serverUrl    = proxySettings.proxy_server_;
    voipProxySettings.userName     = proxySettings.login_;
    voipProxySettings.userPassword = proxySettings.password_;
}

void core::im_container::on_voip_call_message(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    const std::string type = _params.get_value_as_string("type");
    if (!im && type != "voip_reset") {
        return;
    }

    if (type == "voip_call_start") {
        const std::string call_type = _params.get_value_as_string("call_type");
        const std::string mode      = _params.get_value_as_string("mode");
        const std::string contact   = _params.get_value_as_string("contact");

        assert(!contact.empty());
        if (!contact.empty()) {
            core::proxy_settings            proxySettings = g_core->get_proxy_settings();
            voip_manager::VoipProxySettings voipProxySettings;
            fromInternalProxySettings2Voip(proxySettings, voipProxySettings);

            im->on_voip_call_set_proxy(voipProxySettings);
            im->on_voip_call_start(contact, call_type == "video", mode == "attach");
        };
    } else if (type == "voip_add_window") {
        voip_manager::WindowParams windowParams;
        windowParams.hwnd = (void*)(uintptr_t)_params.get_value_as_int64("handle");
        windowParams.isPrimary = _params.get_value_as_bool("mode");
        windowParams.isSystem  = _params.get_value_as_bool("system_mode");
        windowParams.scale     = (float)_params.get_value_as_double("scale");

        windowParams.watermark.bitmap.data = NULL;
        windowParams.watermark.bitmap.size = 0;
        windowParams.watermark.bitmap.w    = 0;
        windowParams.watermark.bitmap.h    = 0;

        windowParams.cameraStatus.bitmap.data = NULL;
        windowParams.cameraStatus.bitmap.size = 0;
        windowParams.cameraStatus.bitmap.w    = 0;
        windowParams.cameraStatus.bitmap.h    = 0;

        core::istream* statusStream = NULL;
        if (_params.is_value_exist("camera_status")) {
            statusStream = _params.get_value_as_stream("camera_status");
            const auto h = _params.get_value_as_int   ("camera_status_height");
            const auto w = _params.get_value_as_int   ("camera_status_width");
            assert(statusStream);
            if (statusStream) {
                const auto stream_size = statusStream->size();
                assert(stream_size);
                if (stream_size > 0 && h > 0 && w > 0) {
                    windowParams.cameraStatus.hwnd = windowParams.hwnd;
                    windowParams.cameraStatus.bitmap.data = (void*)statusStream->read(statusStream->size());
                    windowParams.cameraStatus.bitmap.size = stream_size;
                    windowParams.cameraStatus.bitmap.w = w;
                    windowParams.cameraStatus.bitmap.h = h;
                }
            }
        }

        core::istream* stream = NULL;
        if (_params.is_value_exist("watermark")) {
            stream = _params.get_value_as_stream("watermark");
            const auto h = _params.get_value_as_int   ("watermark_height");
            const auto w = _params.get_value_as_int   ("watermark_width");
            assert(stream);
            if (stream) {
                const auto stream_size = stream->size();
                assert(stream_size);
                if (stream_size > 0 && h > 0 && w > 0) {
                    windowParams.watermark.hwnd = windowParams.hwnd;
                    windowParams.watermark.bitmap.data = (void*)stream->read(stream->size());
                    windowParams.watermark.bitmap.size = stream_size;
                    windowParams.watermark.bitmap.w = w;
                    windowParams.watermark.bitmap.h = h;
                }
            }
        }
        im->on_voip_add_window(windowParams);
        if (stream) {
            stream->reset();
        }
        if (statusStream) {
            statusStream->reset();
        }

    } else if (type == "voip_remove_window") {
        void* hwnd = (void*)(uintptr_t)_params.get_value_as_int64("handle");
        im->on_voip_remove_window(hwnd);
    } else if (type == "voip_call_stop") {
        im->on_voip_call_stop();
    } else if (type == "voip_call_volume_change") {
        const int vol = _params.get_value_as_int("volume");
        im->on_voip_volume_change(vol);
    } else if (type == "audio_playback_mute_switch") {
        im->on_voip_mute_switch();
    } else if (type == "voip_call_media_switch") {
        const bool video = _params.get_value_as_bool("video");
        im->on_voip_switch_media(video);
    } else if (type == "voip_sounds_mute") {
        const bool mute = _params.get_value_as_bool("mute");
        im->on_voip_mute_incoming_call_sounds(mute);
    } else if (type == "voip_call_decline") {
        const std::string mode = _params.get_value_as_string("mode");
        im->on_voip_call_end(_params.get_value_as_string("contact"), mode == "busy");
    } else if (type == "converted_avatar") {
        on_voip_avatar_msg(im, _params);
    } else if (type == "backgroung_update") {
        on_voip_background_msg(im, _params);
    } else if (type == "voip_call_accept") {
        const std::string mode    = _params.get_value_as_string("mode");
        const std::string contact = _params.get_value_as_string("contact");

        assert(!contact.empty());
        if (!contact.empty()) {
            core::proxy_settings            proxySettings = g_core->get_proxy_settings();
            voip_manager::VoipProxySettings voipProxySettings;
            fromInternalProxySettings2Voip(proxySettings, voipProxySettings);

            im->on_voip_call_set_proxy(voipProxySettings);
            im->on_voip_call_accept(contact, mode == "video");
        }
    } else if (type == "device_change") {
        const std::string dev_type = _params.get_value_as_string("dev_type");
        const std::string uid = _params.get_value_as_string("uid");
        im->on_voip_device_changed(dev_type, uid);
    } else if (type == "request_calls") {
        im->on_voip_call_request_calls();
    } else if (type == "update") {
        im->on_voip_update();
    } else if (type == "voip_set_window_offsets") {
        void* hwnd = (void*)(uintptr_t)_params.get_value_as_int64("handle");
        int l = _params.get_value_as_int("left");
        int t = _params.get_value_as_int("top");
        int r = _params.get_value_as_int("right");
        int b = _params.get_value_as_int("bottom");

        im->on_voip_window_set_offsets(hwnd, l, t, r, b);
    } else if (type == "voip_reset") {
        if (!im) {
#ifndef STRIP_VOIP
            voip_manager_.get_voip_manager()->reset();
#endif
        } else {
            im->on_voip_reset();
        }
    } 
	else if ("audio_playback_mute")
	{
		const std::string mode    = _params.get_value_as_string("mute");
		im->on_voip_set_mute(mode == "on");
	}
	else {
        assert(false);
    }
}

void core::im_container::on_voip_background_msg(std::shared_ptr<base_im> im, coll_helper& _params) {
    core::istream* stream = _params.get_value_as_stream("background");
    const auto h          = _params.get_value_as_int("background_height");
    const auto w          = _params.get_value_as_int("background_width");
    void* hwnd            = (void*)(uintptr_t)_params.get_value_as_int("window_handle");

    assert(stream);
    if (stream) {
        const auto stream_size = stream->size();

        assert(stream_size);
        if (stream_size > 0 && h > 0 && w > 0) {
            im->on_voip_window_update_background(hwnd, stream->read(stream->size()), stream_size, w, h);
            stream->reset();
        }
    }
}

void core::im_container::on_voip_avatar_msg(std::shared_ptr<base_im> im, coll_helper& _params) {
    typedef void (base_im::*__loader_func)(const std::string& contact, const unsigned char* data, unsigned size, unsigned h, unsigned w);

    auto __load_avatar = [&_params, &im] (const std::string& prefix, __loader_func func) {
        const std::string contact = _params.get_value_as_string("contact");
        const int size = _params.get_value_as_int("size");

        assert(!contact.empty() && size);
        if (contact.empty() || !size) {
            return;
        }

        assert(!!im);
        if (!im) {
            return;
        }

        if (!_params.is_value_exist((prefix + "avatar").c_str())) {
            return;
        }

        core::istream* stream = _params.get_value_as_stream((prefix + "avatar").c_str());
        const auto h          = _params.get_value_as_int((prefix + "avatar_height").c_str());
        const auto w          = _params.get_value_as_int((prefix + "avatar_width").c_str());

        assert(stream);
        if (stream) {
            const auto stream_size = stream->size();

            assert(stream_size);
            if (stream_size > 0) {
                core::base_im& ptr = *im.get();
                (ptr.*func)(contact, stream->read(stream->size()), stream_size, h, w);
                stream->reset();
            }
        }
    };

    __load_avatar("",                     &base_im::on_voip_user_update_avatar);
    __load_avatar("rem_camera_offline_",  &base_im::on_voip_user_update_avatar_no_video);
    __load_avatar("sign_normal_",         &base_im::on_voip_user_update_avatar_text);
    __load_avatar("sign_header_",         &base_im::on_voip_user_update_avatar_text_header);
    __load_avatar("loc_camera_offline_",  &base_im::on_voip_user_update_avatar_camera_off);
    __load_avatar("loc_camera_disabled_", &base_im::on_voip_user_update_avatar_no_camera);
}

void im_container::on_get_archive_index(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->get_archive_index(_seq, _params.get_value_as_string("contact"), -1, -1);
}


void im_container::on_get_archive_messages_buddies(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    auto ids_list = std::make_shared<core::archive::msgids_list>();

    core::ifptr<core::ihheaders_list> val_headers(_params.get_value_as_hheaders("ids"), false);

    for (auto hdr = val_headers->first(); hdr; hdr = val_headers->next())
        ids_list->push_back(hdr->id_);


    im->get_archive_messages_buddies(_seq, _params.get_value_as_string("contact"), ids_list);
}


void im_container::on_get_archive_messages(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->get_archive_messages(
        _seq,
        _params.get_value_as_string("contact"),
        _params.get_value_as_int64("from"),
        _params.get_value_as_int64("count"));
}

void core::im_container::on_message_typing(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;
    im->send_message_typing(_seq, _params.get_value_as_string("contact"));
}

void core::im_container::on_send_message(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    auto type = message_type::base;

    std::string message;
    if (_params->is_value_exist("message"))
        message = _params.get_value_as_string("message");

    bool is_sms = false;
    if (_params->is_value_exist("is_sms"))
    {
        is_sms = _params.get_value_as_bool("is_sms");
        type = message_type::sms;
    }

    bool is_sticker = false;
    if (_params->is_value_exist("sticker"))
    {
        is_sticker = _params.get_value_as_bool("sticker");
        if (is_sticker)
        {
            std::stringstream ss_message;
            ss_message << "ext:" << _params.get_value_as_int("sticker/set_id") << ":" << "sticker:" << _params.get_value_as_int("sticker/id");

            message = ss_message.str();
        }

        type = message_type::sticker;
    }

    assert(!(is_sms && is_sticker));

    im->send_message_to_contact(
        _seq,
        _params.get_value_as_string("contact"),
        message,
        type,
        "");

    if (core::configuration::get_app_config().is_crash_enabled_ && message == "!crash")
    {
        throw new std::exception();
    }
}

void core::im_container::on_feedback(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    std::string url;
    std::map<std::string, std::string> fields;
    std::vector<std::string> files;

    url = _params.get_value_as_string("url");

    fields.insert(std::make_pair("fb.screen_resolution", _params.get_value_as_string("fb.screen_resolution")));
    fields.insert(std::make_pair("fb.referrer", _params.get_value_as_string("fb.referrer")));
    fields.insert(std::make_pair("fb.question.3004", _params.get_value_as_string("fb.question.3004")));
    fields.insert(std::make_pair("fb.question.159", _params.get_value_as_string("fb.question.159")));
    fields.insert(std::make_pair("fb.question.178", _params.get_value_as_string("fb.question.178")));
    fields.insert(std::make_pair("fb.question.3005", _params.get_value_as_string("fb.question.3005")));
    fields.insert(std::make_pair("fb.user_name", _params.get_value_as_string("fb.user_name")));
    fields.insert(std::make_pair("fb.message", _params.get_value_as_string("fb.message")));
    fields.insert(std::make_pair("fb.communication_email", _params.get_value_as_string("fb.communication_email")));
    fields.insert(std::make_pair("Lang", _params.get_value_as_string("Lang")));
    fields.insert(std::make_pair("attachements_count", _params.get_value_as_string("attachements_count")));

    if (_params.is_value_exist("fb.attachement"))
    {
        core::iarray* array = _params.get_value_as_array("fb.attachement");
        for (int i = 0; i < array->size(); ++i)
        {
            files.push_back(array->get_at(i)->get_as_string());
        }
    }

    im->send_feedback(_seq, url, fields, files);
}

void core::im_container::on_phoneinfo(int64_t seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;
    
    std::string phone = _params.get_value_as_string("phone");
    std::string gui_locale = _params.get_value_as_string("gui_locale");
    
    im->phoneinfo(seq, phone, gui_locale);
}

void core::im_container::on_set_state(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    auto state = profile_state::online;
    std::string sstate = _params.get_value_as_string("state");
    if (sstate == "dnd")
        state = profile_state::dnd;
    else if (sstate == "away")
        state = profile_state::away;
    else if (sstate == "invisible")
        state = profile_state::invisible;
    else if (sstate == "offline")
        state = profile_state::offline;

    im->set_state(_seq, state);
}

void core::im_container::on_remove_members(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->remove_members(
        _seq,
        _params.get_value_as_string("aimid"),
        _params.get_value_as_string("m_chat_members_to_remove"));
}

void core::im_container::on_add_members(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->add_members(
        _seq,
        _params.get_value_as_string("aimid"),
        _params.get_value_as_string("m_chat_members_to_add"));
}

void core::im_container::on_add_chat(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    std::vector<std::string> members;

    auto member_array = _params.get_value_as_array("m_chat_members");
    for (int i = 0; member_array && i < member_array->size(); ++i)
    {
        members.push_back(member_array->get_at(i)->get_as_string());
    }

    im->add_chat(
        _seq,
        _params.get_value_as_string("m_chat_name"),
        members);
}

void core::im_container::on_modify_chat(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->modify_chat(
        _seq,
        _params.get_value_as_string("aimid"), _params.get_value_as_string("m_chat_name"));
}

void core::im_container::on_login_by_password(int64_t _seq, coll_helper& _params)
{
    login_info info;
    info.set_login_type(login_type::lt_login_password);
    info.set_login(_params.get_value_as_string("login"));
    info.set_password(_params.get_value_as_string("password"));
    info.set_save_auth_data(_params.get_value_as_bool("save_auth_data"));

    std::shared_ptr<base_im> im = std::make_shared<wim::im>(im_login_id(""), voip_manager_);
    ims_.clear();
    im->login(_seq, info);
    ims_.push_back(im);
}

void core::im_container::on_login_by_password_for_attach_uin(int64_t _seq, coll_helper& _params)
{
    login_info info;
    info.set_login_type(login_type::lt_login_password);
    info.set_login(_params.get_value_as_string("login"));
    info.set_password(_params.get_value_as_string("password"));
    info.set_save_auth_data(_params.get_value_as_bool("save_auth_data"));

    auto from_im = get_im(_params);
    if (from_im)
        from_im->start_attach_uin(_seq, info, from_im->make_wim_params());
}

void core::im_container::on_login_get_sms_code(int64_t _seq, coll_helper& _params)
{
    phone_info info;
    info.set_phone(_params.get_value_as_string("phone"));

    std::shared_ptr<base_im> im;

    auto is_login = _params.get_value_as_bool("is_login");
    if (is_login)
    {
        ims_.clear();
        im = std::make_shared<wim::im>(im_login_id(""), voip_manager_);
    }
    else
    {
        im = get_im(_params);
    }

    im->login_normalize_phone(_seq, _params.get_value_as_string("country"), _params.get_value_as_string("phone"), _params.get_value_as_string("locale"), is_login);

    if (is_login)
        ims_.push_back(im);
}

void core::im_container::on_login_by_phone(int64_t _seq, coll_helper& _params)
{
    phone_info info;
    info.set_phone(_params.get_value_as_string("phone"));
    info.set_sms_code(_params.get_value_as_string("sms_code"));

    if (ims_.empty())
    {
        assert(!"ims empty");
        return;
    }

    bool is_login = _params.get_value_as_bool("is_login");
    if (is_login)
        (*ims_.begin())->login_by_phone(_seq, info);
    else
        (*ims_.begin())->start_attach_phone(_seq, info);
}

void core::im_container::on_connect_after_migration(int64_t _seq, coll_helper& _params)
{
    create();
}

void core::im_container::on_logout(int64_t _seq, coll_helper& _params)
{
    assert(!ims_.empty());
    if (ims_.empty())
        return;

    std::weak_ptr<core::im_container> wr_this(shared_from_this());

    auto __onlogout = [wr_this] ()
    {
        auto ptr_this = wr_this.lock();
        if (!ptr_this)
            return;

        g_core->start_session_stats();
        g_core->post_message_to_gui("need_login", 0, nullptr);

        (*ptr_this->ims_.begin())->erase_auth_data();

        ptr_this->ims_.clear();
    };

#ifndef STRIP_VOIP
    if (voip_manager_.get_call_manager()->call_get_count())
#else
    if (0)
#endif
    {
        auto __doLogout = [wr_this, __onlogout] ()
        {
            auto ptr_this = wr_this.lock();
            if (!ptr_this)
            {
                return;
            }
            (*ptr_this->ims_.begin())->logout(__onlogout);;
        };
#ifndef STRIP_VOIP
        voip_manager_.get_call_manager()->call_stop_smart(__doLogout);
#endif //STRIP_VOIP
    }
    else
    {
        (*ims_.begin())->logout(__onlogout);
    }
}

void core::im_container::logout()
{
    assert(!ims_.empty());
    if (ims_.empty())
        return;

    g_core->post_message_to_gui("need_login", 0, nullptr);

    ims_.clear();
}

void core::im_container::on_sign_url(int64_t _seq, coll_helper& _params)
{
    (*ims_.begin())->sign_url(_seq, _params.get_value_as_string("url"));
}

void core::im_container::on_stats(int64_t _seq, coll_helper& _params)
{
    core::stats::event_props_type props;

    core::iarray* prop_array = _params.get_value_as_array("props");
    for (int i = 0; prop_array && i < prop_array->size(); ++i)
    {
        core::coll_helper value(prop_array->get_at(i)->get_as_collection(), false);
        auto prop_name = value.get_value_as_string("name");
        auto prop_value = value.get_value_as_string("value");
        props.push_back(std::make_pair(prop_name, prop_value));
    }

    g_core->insert_event((core::stats::stats_event_names)_params.get_value_as_int("event"), props);
}

void core::im_container::on_url_played(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->set_played(_params.get_value_as_string("url"), _params.get_value_as_bool("played"));
}

void core::im_container::on_speech_to_text(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->speech_to_text(_seq, _params.get_value_as_string("url"), _params.get_value_as_string("locale"));
}

std::shared_ptr<base_im> core::im_container::get_im(coll_helper& _params) const
{
    // temporary, for many im
    return get_im_by_id(0);
}

std::shared_ptr<base_im> core::im_container::get_im_by_id(int32_t _id) const
{
    if (ims_.empty())
    {
        return std::shared_ptr<base_im>();
    }

    return (*ims_.begin());
}

void core::im_container::on_get_contact_avatar(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->get_contact_avatar(_seq, _params.get_value_as_string("contact"), _params.get_value_as_int("size"));
}

void core::im_container::on_show_contact_avatar(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->show_contact_avatar(_seq, _params.get_value_as_string("contact"), _params.get_value_as_int("size"));
}

void core::im_container::on_delete_archive_messages(int64_t _seq, coll_helper& _params)
{
    assert(_seq > 0);

    auto im = get_im(_params);
    if (!im)
    {
        return;
    }

    const auto contact_aimid = _params.get<std::string>("contact_aimid");
    assert(!contact_aimid.empty());

    const auto messages_ids = _params.get_value_as_array("messages_ids");
    assert(messages_ids);

    const auto for_all = _params.get<bool>("for_all");

    const auto ids_number = messages_ids->size();
    assert(ids_number > 0);

    std::vector<int64_t> ids;
    ids.reserve(ids_number);

    for (auto index = 0; index < ids_number; ++index)
    {
        const auto id = messages_ids->get_at(index)->get_as_int64();
        assert(id > 0);

        ids.push_back(id);
    }

    im->delete_archive_messages(_seq, contact_aimid, ids, for_all);
}

void core::im_container::on_delete_archive_messages_from(int64_t _seq, coll_helper& _params)
{
    assert(_seq > 0);

    auto im = get_im(_params);
    if (!im)
    {
        return;
    }

    const auto contact_aimid = _params.get<std::string>("contact_aimid");
    assert(!contact_aimid.empty());

    const auto from_id = _params.get<int64_t>("from_id");
    assert(from_id >= 0);

    im->delete_archive_messages_from(_seq, contact_aimid, from_id);
}

void core::im_container::on_add_opened_dialog(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->add_opened_dialog(_params.get_value_as_string("contact"));
}

void core::im_container::on_remove_opened_dialog(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->remove_opened_dialog(_params.get_value_as_string("contact"));
}

void core::im_container::on_set_first_message(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->set_first_message(_params.get_value_as_string("contact"), _params.get_value_as_int64("message"));
}

void core::im_container::on_search(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    std::vector<std::string> searchPatterns;
    core::iarray* searchArray = _params.get_value_as_array("search_patterns");
    for (int i = 0; i < searchArray->size(); ++i)
    {
        core::coll_helper value(searchArray->get_at(i)->get_as_collection(), false);
        searchPatterns.push_back(value.get_value_as_string("pattern"));
    }

    im->search(searchPatterns);
}

void core::im_container::on_set_last_read(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->set_last_read(_params.get_value_as_string("contact"), _params.get_value_as_int64("message"));
}

void im_container::on_hide_chat(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->hide_chat(_params.get_value_as_string("contact"));
}

void im_container::on_mute_chat(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->mute_chat(_params.get_value_as_string("contact"), _params.get_value_as_bool("mute"));
}

void im_container::on_upload_file_sharing(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->upload_file_sharing(
        _seq,
        _params.get_value_as_string("contact"),
        _params.get_value_as_string("file"));
}

void im_container::on_abort_file_sharing_uploading(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->abort_file_sharing_upload(
        _seq,
        _params.get_value_as_string("contact"),
        _params.get_value_as_string("process_seq"));
}

void im_container::on_download_file(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->download_file_sharing(
        _seq,
        _params.get<std::string>("contact"),
        _params.get<std::string>("url"),
        _params.get<std::string>("download_dir"),
        _params.is_value_exist("filename") ? _params.get<std::string>("filename") : std::string(),
        _params.get<file_sharing_function>("function"));
}

void im_container::on_download_image(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->download_image(
        _seq,
        _params.get<std::string>("uri"),
        _params.get<std::string>("destination", ""),
        _params.get<bool>("is_preview"),
        _params.get<int32_t>("preview_height"));
}

void im_container::on_abort_file_downloading(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->abort_file_sharing_download(
        _seq,
        _params.get_value_as_int64("process_seq"));
}

void im_container::on_get_stickers_meta(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->get_stickers_meta(_seq, _params.get_value_as_string("size"));
}

void im_container::on_get_themes_meta(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->get_themes_meta(_seq, (ThemesScale)_params.get_value_as_int("themes_scale"));
}

void im_container::on_get_theme(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->get_theme(_seq, _params.get_value_as_int("theme_id"));
}

void im_container::on_get_sticker(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->get_sticker(_seq,
        _params.get<int32_t>("set_id"),
        _params.get<int32_t>("sticker_id"),
        _params.get<core::sticker_size>("size"));
}

void im_container::on_get_chat_info(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    std::string aimid, stamp;

    if (_params->is_value_exist("stamp"))
    {
        stamp = _params.get<std::string>("stamp");
    }
    else if (_params->is_value_exist("aimid"))
    {
        aimid = _params.get<std::string>("aimid");
    }
    else
    {
        assert(false);
    }

    im->get_chat_info(_seq, aimid, stamp, _params.get<int32_t>("limit"));
}

void im_container::on_get_chat_blocked(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->get_chat_blocked(_seq, _params.get<std::string>("aimid"));
}

void im_container::on_get_chat_home(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;


    std::string tag;
    if (_params.is_value_exist("tag"))
        tag = _params.get<std::string>("tag");
    im->get_chat_home(_seq, tag);
}

void im_container::on_search_contacts(int64_t _seq, coll_helper& _params)
{
    core::search_params filters;
    filters.unserialize(_params);

    auto im = get_im(_params);
    if (!im)
        return;

    im->search_contacts(_seq, filters);
}

void im_container::on_search_contacts2(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;
    im->search_contacts2(_seq, _params.get<std::string>("keyword"), _params.get<std::string>("phonenumber"), _params.get<std::string>("tag"));
}

void im_container::on_profile(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->get_profile(_seq, _params.get_value_as_string("aimid"));
}

void im_container::on_hide_dlg_state(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->hide_dlg_state(_params.get_value_as_string("aimid"));
}

void im_container::on_add_contact(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->add_contact(
        _seq,
        _params.get_value_as_string("contact"),
        _params.get_value_as_string("group"),
        _params.get_value_as_string("message"));
}

void im_container::on_remove_contact(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->remove_contact(
        _seq,
        _params.get_value_as_string("contact"));
}

void im_container::on_rename_contact(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->rename_contact(
        _seq,
        _params.get_value_as_string("contact"),
        _params.get_value_as_string("friendly")
        );
}


void im_container::on_spam_contact(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->spam_contact(
        _seq,
        _params.get_value_as_string("contact"));
}

void im_container::on_ignore_contact(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->ignore_contact(
        _seq,
        _params.get_value_as_string("contact"), _params.get_value_as_bool("ignore"));
}

void im_container::on_get_ignore_contacts(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->get_ignore_list(_seq);
}

void im_container::on_favorite(int64_t _seq, core::coll_helper &_params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->favorite(_params.get_value_as_string("contact"));
}

void im_container::on_unfavorite(int64_t _seq, core::coll_helper &_params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->unfavorite(_params.get_value_as_string("contact"));
}

themes::theme* im_container::get_theme(coll_helper& _params)
{
    auto im = get_im(_params);
    int theme_id = _params.get_value_as_int("id");
    themes::theme *the_theme = im->get_theme_from_cache(theme_id);
    return the_theme;
}

void im_container::on_get_flags(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->load_flags(_seq);
}

void core::im_container::on_update_profile(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    std::vector<std::pair<std::string, std::string>> fields;

    auto field_array = _params.get_value_as_array("fields");
    for (int i = 0; field_array && i < field_array->size(); ++i)
    {
        core::coll_helper value(field_array->get_at(i)->get_as_collection(), false);
        auto field_name = value.get_value_as_string("field_name");
        auto field_value = value.get_value_as_string("field_value");
        fields.push_back(std::make_pair(field_name, field_value));
    }

    im->update_profile(_seq, fields);
}

void core::im_container::on_join_livechat(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    std::string stamp = _params.get_value_as_string("stamp");

    im->join_live_chat(_seq, stamp);
}

void core::im_container::on_set_locale(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    std::string locale = _params.get_value_as_string("locale");

    g_core->set_locale(locale);
}

void core::im_container::on_set_user_proxy(int64_t _seq, coll_helper& _params)
{
    proxy_settings user_proxy;

    user_proxy.proxy_type_ = (int32_t)_params.get_value_as_enum<proxy_types>("settings_proxy_type");

    assert(user_proxy.proxy_type_ >= 0 || user_proxy.proxy_type_ == -1);

    user_proxy.use_proxy_ = user_proxy.proxy_type_ != -1;
    user_proxy.proxy_server_ = tools::from_utf8(_params.get_value_as_string("settings_proxy_server"));
    user_proxy.proxy_port_ = _params.get_value_as_int("settings_proxy_port");
    user_proxy.login_ =  tools::from_utf8(_params.get_value_as_string("settings_proxy_username"));
    user_proxy.password_ =  tools::from_utf8(_params.get_value_as_string("settings_proxy_password"));
    user_proxy.need_auth_ = _params.get_value_as_bool("settings_proxy_need_auth");

    g_core->set_user_proxy_settings(user_proxy);
}

void core::im_container::on_set_avatar(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    tools::binary_stream bs_data;

    core::istream* stream = _params.get_value_as_stream("avatar");
    uint32_t size = stream->size();
    if (stream && size)
    {
        bs_data.write((const char*) stream->read(size), size);
        stream->reset();
    }

    im->set_avatar(_seq, bs_data, _params.is_value_exist("aimid") ? _params.get_value_as_string("aimid") : std::string());
}

void core::im_container::on_mod_chat_name(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->mod_chat_name(_seq, _params.get_value_as_string("aimid"), _params.get_value_as_string("name"));
}

void core::im_container::on_mod_chat_about(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->mod_chat_about(_seq, _params.get_value_as_string("aimid"), _params.get_value_as_string("about"));
}

void core::im_container::on_mod_chat_public(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->mod_chat_public(_seq, _params.get_value_as_string("aimid"), _params.get_value_as_bool("public"));
}

void core::im_container::on_block_chat_member(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->block_chat_member(_seq, _params.get_value_as_string("aimid"), _params.get_value_as_string("contact"), _params.get_value_as_bool("block"));
}

void core::im_container::on_set_chat_member_role(int64_t _seq, coll_helper& _params)
{
    auto im = get_im(_params);
    if (!im)
        return;

    im->set_chat_member_role(_seq, _params.get_value_as_string("aimid"), _params.get_value_as_string("contact"), _params.get_value_as_string("role"));
}

void core::im_container::on_close_promo(int64_t _seq, coll_helper& _params)
{
    for (const auto& im : ims_)
    {
        im->set_show_promo_in_auth(false);
        im->save_auth_to_export([im](){ im->start_after_close_promo(); });
    }
}
