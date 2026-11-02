#include "theme.h"

ThemePalette theme_palette(ThemeMode theme, bool invert_bw) {
  ThemePalette p;

#if defined(PBL_COLOR)
  switch (theme) {
    case THEME_WILDERNESS:
      p.bg = GColorBlack;
      p.panel = GColorDarkCandyAppleRed;
      p.panel_edge = GColorBulgarianRose;
      p.bevel_light = GColorRed;
      p.bevel_dark = GColorBlack;
      p.text = GColorYellow;
      p.text_dim = GColorChromeYellow;
      p.xp_fill = GColorRed;
      p.xp_track = GColorBulgarianRose;
      p.hp = GColorIslamicGreen;
      p.prayer = GColorVividCerulean;
      p.run = GColorChromeYellow;
      break;
    case THEME_DARK:
      p.bg = GColorBlack;
      p.panel = GColorDarkGray;
      p.panel_edge = GColorBlack;
      p.bevel_light = GColorLightGray;
      p.bevel_dark = GColorBlack;
      p.text = GColorWhite;
      p.text_dim = GColorLightGray;
      p.xp_fill = GColorYellow;
      p.xp_track = GColorDarkGray;
      p.hp = GColorGreen;
      p.prayer = GColorCyan;
      p.run = GColorOrange;
      break;
    case THEME_CLASSIC:
    default:
      p.bg = GColorBlack;
      p.panel = GColorWindsorTan;
      p.panel_edge = GColorBrass;
      p.bevel_light = GColorRajah;
      p.bevel_dark = GColorBlack;
      p.text = GColorYellow;
      p.text_dim = GColorChromeYellow;
      p.xp_fill = GColorYellow;
      p.xp_track = GColorDarkGray;
      p.hp = GColorGreen;
      p.prayer = GColorVividCerulean;
      p.run = GColorChromeYellow;
      break;
  }
  (void)invert_bw;
#else
  (void)theme;
  if (invert_bw) {
    p.bg = GColorWhite;
    p.panel = GColorLightGray;
    p.panel_edge = GColorBlack;
    p.bevel_light = GColorWhite;
    p.bevel_dark = GColorBlack;
    p.text = GColorBlack;
    p.text_dim = GColorDarkGray;
    p.xp_fill = GColorBlack;
    p.xp_track = GColorLightGray;
    p.hp = GColorBlack;
    p.prayer = GColorDarkGray;
    p.run = GColorBlack;
  } else {
    p.bg = GColorBlack;
    p.panel = GColorWhite;
    p.panel_edge = GColorWhite;
    p.bevel_light = GColorWhite;
    p.bevel_dark = GColorBlack;
    p.text = GColorWhite;
    p.text_dim = GColorWhite;
    p.xp_fill = GColorWhite;
    p.xp_track = GColorBlack;
    p.hp = GColorWhite;
    p.prayer = GColorWhite;
    p.run = GColorWhite;
  }
#endif

  return p;
}
