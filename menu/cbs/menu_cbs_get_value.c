/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2011-2016 - Daniel De Matteis
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <file/file_path.h>
#include <compat/strl.h>
#include <string/stdstring.h>
#include <lists/string_list.h>

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "../menu_driver.h"
#include "../menu_animation.h"
#include "../menu_cbs.h"
#include "../menu_shader.h"
#include "../menu_display.h"

#include "../../input/input_autodetect.h"
#include "../../input/input_config.h"

#include "../../core.h"
#include "../../core_info.h"
#include "../../configuration.h"
#include "../../file_path_special.h"
#include "../../managers/core_option_manager.h"
#include "../../managers/cheat_manager.h"
#include "../../performance_counters.h"
#include "../../paths.h"
#include "../../runloop.h"
#include "../../intl/intl.h"

#ifndef BIND_ACTION_GET_VALUE
#define BIND_ACTION_GET_VALUE(cbs, name) \
   cbs->action_get_value = name; \
   cbs->action_get_value_ident = #name;
#endif

const char axis_labels[4][128] = {
   RETRO_LBL_ANALOG_LEFT_X,
   RETRO_LBL_ANALOG_LEFT_Y,
   RETRO_LBL_ANALOG_RIGHT_X,
   RETRO_LBL_ANALOG_RIGHT_Y
};

static void menu_action_setting_disp_set_label_cheat_num_passes(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   *w = 19;
   strlcpy(s2, path, len2);
   snprintf(s, len, "%u", cheat_manager_get_buf_size());
}

static void menu_action_setting_disp_set_label_cheevos_locked_entry(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   *w = 19;
   strlcpy(s2, path, len2);
   strlcpy(s,
         msg_hash_to_str(MENU_ENUM_LABEL_VALUE_CHEEVOS_LOCKED_ENTRY), len);
}

static void menu_action_setting_disp_set_label_cheevos_unlocked_entry(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   *w = 19;
   strlcpy(s2, path, len2);
   strlcpy(s,
         msg_hash_to_str(MENU_ENUM_LABEL_VALUE_CHEEVOS_UNLOCKED_ENTRY), len);
}

static void menu_action_setting_disp_set_label_remap_file_load(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   global_t *global = global_get_ptr();

   *w = 19;
   strlcpy(s2, path, len2);
   if (global)
      fill_pathname_base(s, global->name.remapfile,
            len);
}

static void menu_action_setting_disp_set_label_configurations(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   *w = 19;
   strlcpy(s2, path, len2);

   if (!path_is_config_empty())
      fill_pathname_base(s, path_get_config(),
            len);
   else
      strlcpy(s, msg_hash_to_str(MENU_ENUM_LABEL_VALUE_DIRECTORY_DEFAULT), len);
}

static void menu_action_setting_disp_set_label_shader_filter_pass(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   unsigned pass = 0;
#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_HLSL)
   struct video_shader *shader = NULL;
#endif

   (void)pass;

   *s = '\0';
   *w = 19;
   strlcpy(s2, path, len2);

#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_HLSL)
   menu_driver_ctl(RARCH_MENU_CTL_SHADER_GET,
         &shader);
   if (!shader)
      return;

  pass = (type - MENU_SETTINGS_SHADER_PASS_FILTER_0);

  switch (shader->pass[pass].filter)
  {
     case 0:
        strlcpy(s, msg_hash_to_str(MENU_ENUM_LABEL_VALUE_DONT_CARE),
              len);
        break;
     case 1:
        strlcpy(s, msg_hash_to_str(MENU_ENUM_LABEL_VALUE_LINEAR),
              len);
        break;
     case 2:
        strlcpy(s, msg_hash_to_str(MENU_ENUM_LABEL_VALUE_NEAREST),
              len);
        break;
  }
#endif
}

static void menu_action_setting_disp_set_label_filter(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   settings_t *settings = config_get_ptr();

   *s = '\0';
   *w = 19;
   strlcpy(s2, path, len2);
   strlcpy(s, msg_hash_to_str(MENU_ENUM_LABEL_VALUE_NOT_AVAILABLE), len);

   if (settings && *settings->path.softfilter_plugin)
      fill_short_pathname_representation(s,
            settings->path.softfilter_plugin, len);
}

static void menu_action_setting_disp_set_label_pipeline(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   settings_t *settings = config_get_ptr();

   *s = '\0';
   *w = 19;

   switch (settings->menu.xmb.shader_pipeline)
   {
      case XMB_SHADER_PIPELINE_WALLPAPER:
         strlcpy(s, msg_hash_to_str(MENU_ENUM_LABEL_VALUE_OFF), len);
         break;
      case XMB_SHADER_PIPELINE_SIMPLE_RIBBON:
         strlcpy(s, 
               msg_hash_to_str(
                  MENU_ENUM_LABEL_VALUE_SHADER_PIPELINE_RIBBON_SIMPLIFIED), len);
         break;
      case XMB_SHADER_PIPELINE_RIBBON:
         strlcpy(s, 
               msg_hash_to_str(
                  MENU_ENUM_LABEL_VALUE_SHADER_PIPELINE_RIBBON), len);
         break;
   }

   strlcpy(s2, path, len2);

}

static void menu_action_setting_disp_set_label_shader_num_passes(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_HLSL)
   struct video_shader *shader = NULL;
#endif

   *s = '\0';
   *w = 19;
   strlcpy(s2, path, len2);
#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_HLSL)
   menu_driver_ctl(RARCH_MENU_CTL_SHADER_GET,
         &shader);
   if (shader)
      snprintf(s, len, "%u", shader->passes);
#endif
}

static void menu_action_setting_disp_set_label_shader_pass(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_HLSL)
   struct video_shader *shader = NULL;
#endif
   unsigned pass = (type - MENU_SETTINGS_SHADER_PASS_0);

   (void)pass;

   *s = '\0';
   *w = 19;
   strlcpy(s2, path, len2);
   strlcpy(s, msg_hash_to_str(MENU_ENUM_LABEL_VALUE_NOT_AVAILABLE), len);

#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_HLSL)
   menu_driver_ctl(RARCH_MENU_CTL_SHADER_GET,
         &shader);
   if (!shader)
      return;

   if (!string_is_empty(shader->pass[pass].source.path))
      fill_pathname_base(s,
            shader->pass[pass].source.path, len);
#endif
}

static void menu_action_setting_disp_set_label_shader_default_filter(

      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   settings_t *settings = config_get_ptr();

   *s = '\0';
   *w = 19;

   if (!settings)
      return;

   if (settings->video.smooth)
      strlcpy(s, msg_hash_to_str(MENU_ENUM_LABEL_VALUE_LINEAR), len); 
   else
      strlcpy(s, msg_hash_to_str(MENU_ENUM_LABEL_VALUE_NEAREST), len);
}

static void menu_action_setting_disp_set_label_shader_parameter(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_HLSL)
   video_shader_ctx_t shader_info;
   const struct video_shader_parameter *param = NULL;
#endif
   *s = '\0';
   *w = 19;
   strlcpy(s2, path, len2);

#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_HLSL)
   video_shader_driver_get_current_shader(&shader_info);

   if (!shader_info.data)
      return;

   param = &shader_info.data->parameters[type - 
      MENU_SETTINGS_SHADER_PARAMETER_0];

   if (!param)
      return;

   snprintf(s, len, "%.2f [%.2f %.2f]",
         param->current, param->minimum, param->maximum);
#endif
}

static void menu_action_setting_disp_set_label_shader_preset_parameter(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_HLSL)
   const struct video_shader_parameter *param = NULL;
   struct video_shader *shader = NULL;
#endif

   *s = '\0';
   *w = 19;
   strlcpy(s2, path, len2);

#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_HLSL)
   menu_driver_ctl(RARCH_MENU_CTL_SHADER_GET,
         &shader);
   if (!shader)
      return;

   param = &shader->parameters[type - MENU_SETTINGS_SHADER_PRESET_PARAMETER_0];

   if (!param)
      return;

   snprintf(s, len, "%.2f [%.2f %.2f]",
         param->current, param->minimum, param->maximum);
#endif
}

static void menu_action_setting_disp_set_label_shader_scale_pass(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   unsigned pass               = 0;
   unsigned scale_value        = 0;
#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_HLSL)
   struct video_shader *shader = NULL;
#endif

   *s = '\0';
   *w = 19;
   strlcpy(s2, path, len2);

   (void)pass;
   (void)scale_value;

#if defined(HAVE_CG) || defined(HAVE_GLSL) || defined(HAVE_HLSL)
   menu_driver_ctl(RARCH_MENU_CTL_SHADER_GET,
         &shader);
   if (!shader)
      return;

   pass        = (type - MENU_SETTINGS_SHADER_PASS_SCALE_0);
   scale_value = shader->pass[pass].fbo.scale_x;

   if (!scale_value)
      strlcpy(s, msg_hash_to_str(MENU_ENUM_LABEL_VALUE_DONT_CARE), len);
   else
      snprintf(s, len, "%ux", scale_value);
#endif
}

static void menu_action_setting_disp_set_label_menu_file_core(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   const char *alt = NULL;
   strlcpy(s, "(CORE)", len);

   menu_entries_get_at_offset(list, i, NULL,
         NULL, NULL, NULL, &alt);

   *w = strlen(s);
   if (alt)
      strlcpy(s2, alt, len2);
}

static void menu_action_setting_disp_set_label_input_desc(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   char descriptor[PATH_MAX_LENGTH];
   const struct retro_keybind *auto_bind = NULL;
   const struct retro_keybind *keybind   = NULL;
   settings_t *settings                  = config_get_ptr();
   unsigned inp_desc_index_offset        = 
      type - MENU_SETTINGS_INPUT_DESC_BEGIN;
   unsigned inp_desc_user                = inp_desc_index_offset /
      (RARCH_FIRST_CUSTOM_BIND + 4);
   unsigned inp_desc_button_index_offset = inp_desc_index_offset -
      (inp_desc_user * (RARCH_FIRST_CUSTOM_BIND + 4));
   unsigned remap_id                     = 0;

   if (!settings)
      return;
   
   remap_id = settings->input.remap_ids
      [inp_desc_user][inp_desc_button_index_offset];

   keybind = (const struct retro_keybind*)
      &settings->input.binds[inp_desc_user][remap_id];
   auto_bind = (const struct retro_keybind*)
      input_get_auto_bind(inp_desc_user, remap_id);

   input_config_get_bind_string(descriptor,
      keybind, auto_bind, sizeof(descriptor));

   if (inp_desc_button_index_offset < RARCH_FIRST_CUSTOM_BIND)
   {
      if(strstr(descriptor, "Auto") && !strstr(descriptor,
               msg_hash_to_str(MENU_ENUM_LABEL_VALUE_NOT_AVAILABLE)))
         strlcpy(s, 
            descriptor,
            len);
      else
         strlcpy(s, 
            settings->input.binds[inp_desc_user][remap_id].desc,
            len);
   }



   else
      strlcpy(s, axis_labels[remap_id], len);

   *w = 19;
   strlcpy(s2, path, len2);

}

static void menu_action_setting_disp_set_label_cheat(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   unsigned cheat_index = type - MENU_SETTINGS_CHEAT_BEGIN;

   if (cheat_index < cheat_manager_get_buf_size())
      snprintf(s, len, "%s : (%s)",
            (cheat_manager_get_code(cheat_index) != NULL)
            ? cheat_manager_get_code(cheat_index) : 
            msg_hash_to_str(MENU_ENUM_LABEL_VALUE_NOT_AVAILABLE),
            cheat_manager_get_code_state(cheat_index) ? 
            msg_hash_to_str(MENU_ENUM_LABEL_VALUE_ON) :
            msg_hash_to_str(MENU_ENUM_LABEL_VALUE_OFF)
            );
   *w = 19;
   strlcpy(s2, path, len2);
}

static void menu_action_setting_disp_set_label_perf_counters_common(
      struct retro_perf_counter **counters,
      unsigned offset, char *s, size_t len
      )
{
   if (!counters[offset])
      return;
   if (!counters[offset]->call_cnt)
      return;

   snprintf(s, len,
#ifdef _WIN32
         "%I64u ticks, %I64u runs.",
#else
         "%llu ticks, %llu runs.",
#endif
         ((unsigned long long)counters[offset]->total /
          (unsigned long long)counters[offset]->call_cnt),
         (unsigned long long)counters[offset]->call_cnt);
}

static void general_disp_set_label_perf_counters(
      struct retro_perf_counter **counters,
      unsigned offset,
      char *s, size_t len,
      char *s2, size_t len2,
      const char *path, unsigned *w
      )
{
   *s = '\0';
   *w = 19;
   strlcpy(s2, path, len2);

   menu_action_setting_disp_set_label_perf_counters_common(
         counters, offset, s, len);
   menu_animation_ctl(MENU_ANIMATION_CTL_SET_ACTIVE, NULL);
}

static void menu_action_setting_disp_set_label_perf_counters(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   struct retro_perf_counter **counters = retro_get_perf_counter_rarch();
   unsigned offset = type - MENU_SETTINGS_PERF_COUNTERS_BEGIN;
   general_disp_set_label_perf_counters(counters, offset, s, len,
         s2, len, path, w);
}

static void menu_action_setting_disp_set_label_libretro_perf_counters(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   struct retro_perf_counter **counters = retro_get_perf_counter_libretro();
   unsigned offset = type - MENU_SETTINGS_LIBRETRO_PERF_COUNTERS_BEGIN;
   general_disp_set_label_perf_counters(counters, offset, s, len,
         s2, len, path, w);
}

static void menu_action_setting_disp_set_label_menu_more(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   strlcpy(s, msg_hash_to_str(MENU_ENUM_LABEL_VALUE_MORE), len);
   *w = 19;
   strlcpy(s2, path, len2);
}

static void menu_action_setting_disp_set_label_state(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   settings_t *settings        = config_get_ptr();

   if (!settings)
      return;

   strlcpy(s2, path, len2);
   *w = 16;
   snprintf(s, len, "%d", settings->state_slot);
   if (settings->state_slot == -1)
      strlcat(s, " (Auto)", len);
}

static void menu_action_setting_disp_set_label_poll_type_behavior(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   settings_t *settings        = config_get_ptr();

   if (!settings)
      return;

   strlcpy(s2, path, len2);
   *w = 19;
   switch (settings->input.poll_type_behavior)
   {
      case 0:
         strlcpy(s,
               msg_hash_to_str(
                  MENU_ENUM_LABEL_VALUE_INPUT_POLL_TYPE_BEHAVIOR_EARLY), len);
         break;
      case 1:
         strlcpy(s,
               msg_hash_to_str(
                  MENU_ENUM_LABEL_VALUE_INPUT_POLL_TYPE_BEHAVIOR_NORMAL), len);
         break;
      case 2:
         strlcpy(s,
               msg_hash_to_str(
                  MENU_ENUM_LABEL_VALUE_INPUT_POLL_TYPE_BEHAVIOR_LATE), len);
         break;
   }
}

static void menu_action_setting_disp_set_label_xmb_theme(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   settings_t *settings        = config_get_ptr();

   if (!settings)
      return;

   strlcpy(s2, path, len2);
   *w = 19;
   switch (settings->menu.xmb.theme)
   {
      case XMB_ICON_THEME_MONOCHROME:
         strlcpy(s,
               msg_hash_to_str(MENU_ENUM_LABEL_VALUE_XMB_ICON_THEME_MONOCHROME), len);
         break;
      case XMB_ICON_THEME_FLATUI:
         strlcpy(s,
               msg_hash_to_str(MENU_ENUM_LABEL_VALUE_XMB_ICON_THEME_FLATUI), len);
         break;
      case XMB_ICON_THEME_RETROACTIVE:
         strlcpy(s,
               msg_hash_to_str(MENU_ENUM_LABEL_VALUE_XMB_ICON_THEME_RETROACTIVE), len);
         break;
      case XMB_ICON_THEME_PIXEL:
         strlcpy(s,
               msg_hash_to_str(MENU_ENUM_LABEL_VALUE_XMB_ICON_THEME_PIXEL), len);
         break;
      case XMB_ICON_THEME_CUSTOM:
         strlcpy(s,
               msg_hash_to_str(MENU_ENUM_LABEL_VALUE_XMB_ICON_THEME_CUSTOM), len);
         break;
   }
}

static void menu_action_setting_disp_set_label_xmb_menu_color_theme(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   settings_t *settings        = config_get_ptr();

   strlcpy(s2, path, len2);
   *w = 19;

   if (!settings)
      return;

   switch (settings->menu.xmb.menu_color_theme)
   {
      case XMB_THEME_WALLPAPER:
         strlcpy(s,
               msg_hash_to_str(
                  MENU_ENUM_LABEL_VALUE_XMB_MENU_COLOR_THEME_PLAIN),
               len);
         break;
      case XMB_THEME_LEGACY_RED:
         strlcpy(s,
               msg_hash_to_str(
                 MENU_ENUM_LABEL_VALUE_XMB_MENU_COLOR_THEME_LEGACY_RED),
               len);
         break;
      case XMB_THEME_DARK_PURPLE:
         strlcpy(s,
               msg_hash_to_str(
                 MENU_ENUM_LABEL_VALUE_XMB_MENU_COLOR_THEME_DARK_PURPLE),
               len);
         break;
      case XMB_THEME_MIDNIGHT_BLUE:
         strlcpy(s,
               msg_hash_to_str(
                 MENU_ENUM_LABEL_VALUE_XMB_MENU_COLOR_THEME_MIDNIGHT_BLUE),
               len);
         break;
      case XMB_THEME_GOLDEN:
         strlcpy(s,
               msg_hash_to_str(
                 MENU_ENUM_LABEL_VALUE_XMB_MENU_COLOR_THEME_GOLDEN),
               len);
         break;
      case XMB_THEME_ELECTRIC_BLUE:
         strlcpy(s,
               msg_hash_to_str(
                 MENU_ENUM_LABEL_VALUE_XMB_MENU_COLOR_THEME_ELECTRIC_BLUE),
               len);
         break;
      case XMB_THEME_APPLE_GREEN:
         strlcpy(s,
               msg_hash_to_str(
                 MENU_ENUM_LABEL_VALUE_XMB_MENU_COLOR_THEME_APPLE_GREEN),
               len);
         break;
      case XMB_THEME_UNDERSEA:
         strlcpy(s,
               msg_hash_to_str(
                 MENU_ENUM_LABEL_VALUE_XMB_MENU_COLOR_THEME_UNDERSEA),
               len);
         break;
      case XMB_THEME_VOLCANIC_RED:
         strlcpy(s,
               msg_hash_to_str(
                 MENU_ENUM_LABEL_VALUE_XMB_MENU_COLOR_THEME_VOLCANIC_RED),
               len);
         break;
      case XMB_THEME_DARK:
         strlcpy(s,
               msg_hash_to_str(
                 MENU_ENUM_LABEL_VALUE_XMB_MENU_COLOR_THEME_DARK),
               len);
         break;
   }
}

static void menu_action_setting_disp_set_label_materialui_menu_color_theme(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   settings_t *settings        = config_get_ptr();

   strlcpy(s2, path, len2);
   *w = 19;

   if (!settings)
      return;

   switch (settings->menu.materialui.menu_color_theme)
   {
      case MATERIALUI_THEME_BLUE:
         strlcpy(s,
               msg_hash_to_str(
                  MENU_ENUM_LABEL_VALUE_MATERIALUI_MENU_COLOR_THEME_BLUE), len);
         break;
      case MATERIALUI_THEME_BLUE_GREY:
         strlcpy(s,
               msg_hash_to_str(
                  MENU_ENUM_LABEL_VALUE_MATERIALUI_MENU_COLOR_THEME_BLUE_GREY), len);
         break;
      case MATERIALUI_THEME_GREEN:
         strlcpy(s,
               msg_hash_to_str(
                  MENU_ENUM_LABEL_VALUE_MATERIALUI_MENU_COLOR_THEME_GREEN), len);
         break;
      case MATERIALUI_THEME_RED:
         strlcpy(s,
               msg_hash_to_str(
                  MENU_ENUM_LABEL_VALUE_MATERIALUI_MENU_COLOR_THEME_RED), len);
         break;
      case MATERIALUI_THEME_YELLOW:
         strlcpy(s,
               msg_hash_to_str(
                  MENU_ENUM_LABEL_VALUE_MATERIALUI_MENU_COLOR_THEME_YELLOW), len);
         break;
      case MATERIALUI_THEME_DARK_BLUE:
         strlcpy(s,
               msg_hash_to_str(
                  MENU_ENUM_LABEL_VALUE_MATERIALUI_MENU_COLOR_THEME_DARK_BLUE), len);
         break;
      case MATERIALUI_THEME_NVIDIA_SHIELD:
         strlcpy(s,
               msg_hash_to_str(
                  MENU_ENUM_LABEL_VALUE_MATERIALUI_MENU_COLOR_THEME_NVIDIA_SHIELD), len);
         break;
      default:
         break;
   }
}

static void menu_action_setting_disp_set_label_thumbnails(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   settings_t *settings        = config_get_ptr();

   if (!settings)
      return;

   strlcpy(s2, path, len2);
   *w = 19;

   switch (settings->menu.thumbnails)
   {
      case 0:
         strlcpy(s, msg_hash_to_str(
                  MENU_ENUM_LABEL_VALUE_OFF), len);
         break;
      case 1:
         strlcpy(s,
               msg_hash_to_str(
                  MENU_ENUM_LABEL_VALUE_THUMBNAIL_MODE_SCREENSHOTS), len);
         break;
      case 2:
         strlcpy(s, 
               msg_hash_to_str(
                  MENU_ENUM_LABEL_VALUE_THUMBNAIL_MODE_TITLE_SCREENS), len);
         break;
      case 3:
         strlcpy(s,
               msg_hash_to_str(
                  MENU_ENUM_LABEL_VALUE_THUMBNAIL_MODE_BOXARTS), len);
         break;
   }
}

static void menu_action_setting_disp_set_label_menu_toggle_gamepad_combo(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   settings_t *settings        = config_get_ptr();

   if (!settings)
      return;

   strlcpy(s2, path, len2);
   *w = 19;

   switch (settings->input.menu_toggle_gamepad_combo)
   {
      case INPUT_TOGGLE_NONE:
         strlcpy(s, msg_hash_to_str(MENU_ENUM_LABEL_VALUE_NONE), len);
         break;
      case INPUT_TOGGLE_DOWN_Y_L_R:
         strlcpy(s, "Down + L1 + R1 + Y", len);
         break;
      case INPUT_TOGGLE_L3_R3:
         strlcpy(s, "L3 + R3", len);
         break;
      case INPUT_TOGGLE_L1_R1_START_SELECT:
         strlcpy(s, "L1 + R1 + Start + Select", len);
         break;
      case INPUT_TOGGLE_START_SELECT:
         strlcpy(s, "Start + Select", len);
         break;
   }
}


static void menu_action_setting_disp_set_label_menu_disk_index(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   unsigned images = 0, current                = 0;
   rarch_system_info_t *system                 = NULL;
   struct retro_disk_control_callback *control = NULL;

   runloop_ctl(RUNLOOP_CTL_SYSTEM_INFO_GET, &system);
   if (!system)
      return;

   control = &system->disk_control_cb;

   if (!control)
      return;

   *w = 19;
   *s = '\0';
   strlcpy(s2, path, len2);

   if (!control->get_num_images)
      return;
   if (!control->get_image_index)
      return;

   images  = control->get_num_images();
   current = control->get_image_index();

   if (current >= images)
      strlcpy(s, msg_hash_to_str(MENU_ENUM_LABEL_VALUE_NO_DISK), len);
   else
      snprintf(s, len, "%u", current + 1);
}

static void menu_action_setting_disp_set_label_menu_input_keyboard_gamepad_mapping_type(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   settings_t         *settings = config_get_ptr();
   unsigned width = 0, height = 0;

   *w = 19;
   *s = '\0';

   (void)width;
   (void)height;

   strlcpy(s2, path, len2);

   switch (settings->input.keyboard_gamepad_mapping_type)
   {
      case 0:
         strlcpy(s, msg_hash_to_str(MENU_ENUM_LABEL_VALUE_NONE), len);
         break;
      case 1:
         strlcpy(s, "iPega PG-9017", len);
         break;
      case 2:
         strlcpy(s, "8-bitty", len);
         break;
      case 3:
         strlcpy(s, "SNES30 8bitdo", len);
         break;
   }
}

static void menu_action_setting_disp_set_label_menu_video_resolution(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   unsigned width = 0, height = 0;

   *w = 19;
   *s = '\0';

   strlcpy(s2, path, len2);

   if (video_driver_get_video_output_size(&width, &height))
   {
#ifdef GEKKO
      if (width == 0 || height == 0)
         strlcpy(s, "DEFAULT", len);
      else
#endif
         snprintf(s, len, "%ux%u", width, height);
   }
   else
      strlcpy(s, msg_hash_to_str(MENU_ENUM_LABEL_VALUE_NOT_AVAILABLE), len);
}

static void menu_action_setting_generic_disp_set_label(
      unsigned *w, char *s, size_t len,
      const char *path, const char *label,
      char *s2, size_t len2)
{
   *s = '\0';

   if (label)
      strlcpy(s, label, len);
   *w = strlen(s);

   strlcpy(s2, path, len2);
}

static void menu_action_setting_disp_set_label_menu_file_plain(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, "(FILE)", s2, len2);
}

static void menu_action_setting_disp_set_label_menu_file_image(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, "(IMG)", s2, len2);
}

static void menu_action_setting_disp_set_label_menu_file_imageviewer(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, "(IMAGE)", s2, len2);
}

static void menu_action_setting_disp_set_label_movie(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, "(MOVIE)", s2, len2);
}

static void menu_action_setting_disp_set_label_music(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, "(MUSIC)", s2, len2);
}

static void menu_action_setting_disp_set_label_menu_file_use_directory(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, NULL, s2, len2);
}

static void menu_action_setting_disp_set_label_menu_file_directory(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, "(DIR)", s2, len2);
}

static void menu_action_setting_disp_set_label_menu_file_parent_directory(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, NULL, s2, len2);
}

static void menu_action_setting_disp_set_label_menu_file_carchive(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, "(COMP)", s2, len2);
}

static void menu_action_setting_disp_set_label_menu_file_shader(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, "(SHADER)", s2, len2);
}

static void menu_action_setting_disp_set_label_menu_file_shader_preset(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, "(PRESET)", s2, len2);
}

static void menu_action_setting_disp_set_label_menu_file_in_carchive(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, "(CFILE)", s2, len2);
}

static void menu_action_setting_disp_set_label_menu_file_overlay(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, "(OVERLAY)", s2, len2);
}

static void menu_action_setting_disp_set_label_menu_file_config(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, "(CONFIG)", s2, len2);
}

static void menu_action_setting_disp_set_label_menu_file_font(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, "(FONT)", s2, len2);
}

static void menu_action_setting_disp_set_label_menu_file_filter(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, "(FILTER)", s2, len2);
}

static void menu_action_setting_disp_set_label_menu_file_url_core(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   const char *alt = NULL;
   strlcpy(s, "(CORE)", len);

   menu_entries_get_at_offset(list, i, NULL,
         NULL, NULL, NULL, &alt);

   *w = strlen(s);
   if (alt)
      strlcpy(s2, alt, len2);
}

static void menu_action_setting_disp_set_label_menu_file_rdb(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, "(RDB)", s2, len2);
}

static void menu_action_setting_disp_set_label_menu_file_cursor(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, "(CURSOR)", s2, len2);
}

static void menu_action_setting_disp_set_label_menu_file_cheat(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   menu_action_setting_generic_disp_set_label(w, s, len,
         path, "(CHEAT)", s2, len2);
}

static void menu_action_setting_disp_set_label_core_option_create(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   rarch_system_info_t *system = NULL;
   global_t            *global = global_get_ptr();

   runloop_ctl(RUNLOOP_CTL_SYSTEM_INFO_GET, &system);
   if (!system)
      return;

   *s = '\0';
   *w = 19;

   strlcpy(s, "", len);

   if (!string_is_empty(global->name.base))
      strlcpy(s,  path_basename(global->name.base), len);

   strlcpy(s2, path, len2);
}

static void menu_action_setting_disp_set_label_playlist_associations(file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   char playlist_name_with_ext[PATH_MAX_LENGTH] = {0};
   bool found_matching_core_association         = false;
   settings_t         *settings                 = config_get_ptr();
   struct string_list *str_list                 = string_split(settings->playlist_names, ";");
   struct string_list *str_list2                = string_split(settings->playlist_cores, ";");

   strlcpy(s2, path, len2);

   *s = '\0';
   *w = 19;

   fill_pathname_noext(playlist_name_with_ext, path,
         file_path_str(FILE_PATH_LPL_EXTENSION),
         sizeof(playlist_name_with_ext));

   for (i = 0; i < str_list->size; i++)
   {
      if (string_is_equal(str_list->elems[i].data, playlist_name_with_ext))
      {
         if (str_list->size != str_list2->size)
            break;

         if (str_list2->elems[i].data == NULL)
            break;

         found_matching_core_association = true;
         strlcpy(s, str_list2->elems[i].data, len);
      }
   }

   string_list_free(str_list);
   string_list_free(str_list2);

   if (string_is_equal(s, file_path_str(FILE_PATH_DETECT)) || !found_matching_core_association)
      strlcpy(s, msg_hash_to_str(MENU_ENUM_LABEL_VALUE_NOT_AVAILABLE), len);
   else
   {
      char buf[PATH_MAX_LENGTH];
      core_info_list_t *list = NULL;

      core_info_get_list(&list);

      if (core_info_list_get_display_name(list, s, buf, sizeof(buf)))
         strlcpy(s, buf, len);
   }

   strlcpy(s2, path, len2);
}

static void menu_action_setting_disp_set_label_core_options(file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   core_option_manager_t *coreopts = NULL;
   const char *core_opt = NULL;

   *s = '\0';
   *w = 19;

   if (runloop_ctl(RUNLOOP_CTL_CORE_OPTIONS_LIST_GET, &coreopts))
   {
      core_opt = core_option_manager_get_val(coreopts,
            type - MENU_SETTINGS_CORE_OPTION_START);

      strlcpy(s, "", len);

      if (core_opt)
         strlcpy(s, core_opt, len);
   }

   strlcpy(s2, path, len2);
}

static void menu_action_setting_disp_set_label_content_history(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   *s = '\0';
   *w = 19;

   menu_setting_get_label(list, s,
         len, w, type, label, entry_label, i);

   strlcpy(s2, path, len2);
}

static void menu_action_setting_disp_set_label_system_information(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   *s = '\0';
   *w = 2;

   menu_setting_get_label(list, s,
         len, w, type, label, entry_label, i);

   strlcpy(s2, path, len2);
}

static void menu_action_setting_disp_set_label_achievement_information(
      file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   *s = '\0';
   *w = 2;

   menu_setting_get_label(list, s,
         len, w, type, label, entry_label, i);

   strlcpy(s2, path, len2);
}

static void menu_action_setting_disp_set_label(file_list_t* list,
      unsigned *w, unsigned type, unsigned i,
      const char *label,
      char *s, size_t len,
      const char *entry_label,
      const char *path,
      char *s2, size_t len2)
{
   *s = '\0';
   *w = 19;

   menu_setting_get_label(list, s,
         len, w, type, label, entry_label, i);

   strlcpy(s2, path, len2);
}

static int menu_cbs_init_bind_get_string_representation_compare_label(
      menu_file_list_cbs_t *cbs)
{
   if (cbs->enum_idx != MSG_UNKNOWN)
   {
      switch (cbs->enum_idx)
      {
         case MENU_ENUM_LABEL_STATE_SLOT:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_state);
            break;
         case MENU_ENUM_LABEL_INPUT_POLL_TYPE_BEHAVIOR:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_poll_type_behavior);
            break;
         case MENU_ENUM_LABEL_XMB_THEME:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_xmb_theme);
            break;
         case MENU_ENUM_LABEL_XMB_MENU_COLOR_THEME:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_xmb_menu_color_theme);
            break;
         case MENU_ENUM_LABEL_MATERIALUI_MENU_COLOR_THEME:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_materialui_menu_color_theme);
            break;
         case MENU_ENUM_LABEL_THUMBNAILS:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_thumbnails);
            break;
         case MENU_ENUM_LABEL_INPUT_MENU_ENUM_TOGGLE_GAMEPAD_COMBO:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_menu_toggle_gamepad_combo);
            break;
         case MENU_ENUM_LABEL_CHEAT_NUM_PASSES:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_cheat_num_passes);
            break;
         case MENU_ENUM_LABEL_REMAP_FILE_LOAD:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_remap_file_load);
            break;
         case MENU_ENUM_LABEL_VIDEO_SHADER_FILTER_PASS:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_shader_filter_pass);
            break;
         case MENU_ENUM_LABEL_VIDEO_SHADER_SCALE_PASS:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_shader_scale_pass);
            break;
         case MENU_ENUM_LABEL_VIDEO_SHADER_NUM_PASSES:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_shader_num_passes);
            break;
         case MENU_ENUM_LABEL_XMB_RIBBON_ENABLE:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_pipeline);
            break;
         case MENU_ENUM_LABEL_VIDEO_SHADER_PASS:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_shader_pass);
            break;
         case MENU_ENUM_LABEL_VIDEO_SHADER_DEFAULT_FILTER:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_shader_default_filter);
            break;
         case MENU_ENUM_LABEL_VIDEO_FILTER:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_filter);
            break;
         case MENU_ENUM_LABEL_CONFIGURATIONS:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_configurations);
            break;
         case MENU_ENUM_LABEL_SCREEN_RESOLUTION:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_menu_video_resolution);
            break;
         case MENU_ENUM_LABEL_INPUT_KEYBOARD_GAMEPAD_MAPPING_TYPE:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_menu_input_keyboard_gamepad_mapping_type);
            break;
         case MENU_ENUM_LABEL_CONTENT_COLLECTION_LIST:
         case MENU_ENUM_LABEL_LOAD_CONTENT_HISTORY:
         case MENU_ENUM_LABEL_DOWNLOADED_FILE_DETECT_CORE_LIST:
         case MENU_ENUM_LABEL_DETECT_CORE_LIST:
         case MENU_ENUM_LABEL_LOAD_CONTENT:
         case MENU_ENUM_LABEL_CORE_OPTIONS:
         case MENU_ENUM_LABEL_CORE_CHEAT_OPTIONS:
         case MENU_ENUM_LABEL_SHADER_OPTIONS:
         case MENU_ENUM_LABEL_VIDEO_SHADER_PARAMETERS:
         case MENU_ENUM_LABEL_VIDEO_SHADER_PRESET_PARAMETERS:
         case MENU_ENUM_LABEL_VIDEO_SHADER_PRESET_SAVE_AS:
         case MENU_ENUM_LABEL_CHEAT_FILE_SAVE_AS:
         case MENU_ENUM_LABEL_FRONTEND_COUNTERS:
         case MENU_ENUM_LABEL_CORE_COUNTERS:
         case MENU_ENUM_LABEL_DATABASE_MANAGER_LIST:
         case MENU_ENUM_LABEL_CURSOR_MANAGER_LIST:
         case MENU_ENUM_LABEL_RESTART_CONTENT:
         case MENU_ENUM_LABEL_CLOSE_CONTENT:
         case MENU_ENUM_LABEL_RESUME_CONTENT:
         case MENU_ENUM_LABEL_TAKE_SCREENSHOT:
         case MENU_ENUM_LABEL_CORE_INPUT_REMAPPING_OPTIONS:
         case MENU_ENUM_LABEL_CORE_INFORMATION:
         case MENU_ENUM_LABEL_SYSTEM_INFORMATION:
         case MENU_ENUM_LABEL_ACHIEVEMENT_LIST:
         case MENU_ENUM_LABEL_SAVE_STATE:
         case MENU_ENUM_LABEL_LOAD_STATE:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_menu_more);
            break;
         default:
            return - 1;
      }
   }
   else
   {
      return -1;
   }

   return 0;
}

static int menu_cbs_init_bind_get_string_representation_compare_type(
      menu_file_list_cbs_t *cbs, unsigned type)
{
   if (type >= MENU_SETTINGS_INPUT_DESC_BEGIN
         && type <= MENU_SETTINGS_INPUT_DESC_END)
   {
      BIND_ACTION_GET_VALUE(cbs,
         menu_action_setting_disp_set_label_input_desc);
   }
   else if (type >= MENU_SETTINGS_CHEAT_BEGIN
         && type <= MENU_SETTINGS_CHEAT_END)
   {
      BIND_ACTION_GET_VALUE(cbs,
         menu_action_setting_disp_set_label_cheat);
   }
   else if (type >= MENU_SETTINGS_PERF_COUNTERS_BEGIN
         && type <= MENU_SETTINGS_PERF_COUNTERS_END)
   {
      BIND_ACTION_GET_VALUE(cbs,
         menu_action_setting_disp_set_label_perf_counters);
   }
   else if (type >= MENU_SETTINGS_LIBRETRO_PERF_COUNTERS_BEGIN
         && type <= MENU_SETTINGS_LIBRETRO_PERF_COUNTERS_END)
   {
      BIND_ACTION_GET_VALUE(cbs,
         menu_action_setting_disp_set_label_libretro_perf_counters);
   }
   else
   {
      switch (type)
      {
         case MENU_SETTINGS_CORE_OPTION_CREATE:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_core_option_create);
            break;
         case FILE_TYPE_CORE:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_core);
            break;
         case FILE_TYPE_PLAIN:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_plain);
            break;
         case FILE_TYPE_MOVIE:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_movie);
            break;
         case FILE_TYPE_MUSIC:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_music);
            break;
         case FILE_TYPE_IMAGE:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_image);
            break;
         case FILE_TYPE_IMAGEVIEWER:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_imageviewer);
            break;
         case FILE_TYPE_USE_DIRECTORY:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_use_directory);
            break;
         case FILE_TYPE_DIRECTORY:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_directory);
            break;
         case FILE_TYPE_PARENT_DIRECTORY:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_parent_directory);
            break;
         case FILE_TYPE_CARCHIVE:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_carchive);
            break;
         case FILE_TYPE_OVERLAY:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_overlay);
            break;
         case FILE_TYPE_FONT:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_font);
            break;
         case FILE_TYPE_SHADER:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_shader);
            break;
         case FILE_TYPE_SHADER_PRESET:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_shader_preset);
            break;
         case FILE_TYPE_CONFIG:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_config);
            break;
         case FILE_TYPE_IN_CARCHIVE:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_in_carchive);
            break;
         case FILE_TYPE_VIDEOFILTER:
         case FILE_TYPE_AUDIOFILTER:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_filter);
            break;
         case FILE_TYPE_DOWNLOAD_CORE:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_url_core);
            break;
         case FILE_TYPE_RDB:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_rdb);
            break;
         case FILE_TYPE_CURSOR:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_cursor);
            break;
         case FILE_TYPE_CHEAT:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_file_cheat);
            break;
         case MENU_SETTING_SUBGROUP:
         case MENU_SETTINGS_CUSTOM_BIND_ALL:
         case MENU_SETTINGS_CUSTOM_BIND_DEFAULT_ALL:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_more);
            break;
         case MENU_SETTINGS_CORE_DISK_OPTIONS_DISK_INDEX:
            BIND_ACTION_GET_VALUE(cbs,
               menu_action_setting_disp_set_label_menu_disk_index);
            break;
         default:
            BIND_ACTION_GET_VALUE(cbs, menu_action_setting_disp_set_label);
            break;
      }
   }

   return 0;
}

int menu_cbs_init_bind_get_string_representation(menu_file_list_cbs_t *cbs,
      const char *path, const char *label, unsigned type, size_t idx)
{
   if (!cbs)
      return -1;

   if (cbs->enum_idx != MSG_UNKNOWN)
   {
      switch (cbs->enum_idx)
      {
         case MENU_ENUM_LABEL_CHEEVOS_UNLOCKED_ENTRY:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_cheevos_unlocked_entry);
            return 0;
         case MENU_ENUM_LABEL_CHEEVOS_LOCKED_ENTRY:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_cheevos_locked_entry);
            return 0;
         case MENU_ENUM_LABEL_LOAD_CONTENT_HISTORY:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_content_history);
            return 0;
         case MENU_ENUM_LABEL_SYSTEM_INFORMATION:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_system_information);
            return 0;
         case MENU_ENUM_LABEL_ACHIEVEMENT_LIST:
            BIND_ACTION_GET_VALUE(cbs,
                  menu_action_setting_disp_set_label_achievement_information);
            return 0;
         default:
            break;
      }
   }

   if (type >= MENU_SETTINGS_PLAYLIST_ASSOCIATION_START)
   {
      BIND_ACTION_GET_VALUE(cbs,
         menu_action_setting_disp_set_label_playlist_associations);
      return 0;
   }
   if (type >= MENU_SETTINGS_CORE_OPTION_START)
   {
      BIND_ACTION_GET_VALUE(cbs,
         menu_action_setting_disp_set_label_core_options);
      return 0;
   }

   if (type >= MENU_SETTINGS_SHADER_PARAMETER_0
         && type <= MENU_SETTINGS_SHADER_PARAMETER_LAST)
   {
      BIND_ACTION_GET_VALUE(cbs,
         menu_action_setting_disp_set_label_shader_parameter);
      return 0;
   }
   else if (type >= MENU_SETTINGS_SHADER_PRESET_PARAMETER_0
         && type <= MENU_SETTINGS_SHADER_PRESET_PARAMETER_LAST)
   {
      BIND_ACTION_GET_VALUE(cbs,
         menu_action_setting_disp_set_label_shader_preset_parameter);
      return 0;
   }

   if (menu_cbs_init_bind_get_string_representation_compare_label(cbs) == 0)
      return 0;

   if (menu_cbs_init_bind_get_string_representation_compare_type(cbs, type) == 0)
      return 0;

   return -1;
}
