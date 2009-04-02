/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_CONFIG_H_
#define ZYPPER_CONFIG_H_

#include <string>

#include "utils/colors.h"

class ConfigOption
{
public:
  static const ConfigOption SOLVER_INSTALL_RECOMMENDS;

  static const ConfigOption COLOR_USE_COLORS;
  static const ConfigOption COLOR_BACKGROUND;
  static const ConfigOption COLOR_RESULT;
  static const ConfigOption COLOR_MSG_STATUS;
  static const ConfigOption COLOR_MSG_ERROR;
  static const ConfigOption COLOR_MSG_WARNING;
  static const ConfigOption COLOR_POSITIVE;
  static const ConfigOption COLOR_NEGATIVE;
  static const ConfigOption COLOR_PROMPT_OPTION;
  static const ConfigOption COLOR_PROMPT_SHORTHAND;

  enum Option
  {
    SOLVER_INSTALL_RECOMMENDS_e,

    COLOR_USE_COLORS_e,
    COLOR_BACKGROUND_e,
    COLOR_RESULT_e,
    COLOR_MSG_STATUS_e,
    COLOR_MSG_ERROR_e,
    COLOR_MSG_WARNING_e,
    COLOR_POSITIVE_e,
    COLOR_NEGATIVE_e,
    COLOR_PROMPT_OPTION_e,
    COLOR_PROMPT_SHORTHAND_e
  };

  ConfigOption(Option option) : _value(option) {}

  explicit ConfigOption(const std::string & strval_r);

  Option toEnum() const { return _value; }

  ConfigOption::Option parse(const std::string & strval_r);

  const std::string asString() const;

private:
  Option _value;
};


/**
 *
 */
struct Config
{
  /** Initializes the config options to defaults. */
  Config();

  /** Reads zypper.conf and stores the result */
  void read();


  bool solver_installRecommends;

  /**
   * Whether to colorize the output. This is evaluated according to
   * color_useColors and has_colors()
   */
  bool do_colors;

  /** zypper.conf: color.useColors */
  std::string color_useColors;

  /**
   * zypper.conf: color.background
   * dark (false) or light (true)
   */
  bool color_background;

  Color color_result;
  Color color_msgStatus;
  Color color_msgError;
  Color color_msgWarning;
  Color color_positive;
  Color color_negative;
  Color color_promptOption;
};

#endif /* ZYPPER_CONFIG_H_ */