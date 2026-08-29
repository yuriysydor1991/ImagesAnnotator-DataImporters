#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "IADataImportersFacade.h"
#include "src/log/ILogger.h"
#include "src/log/default-logger/real-default-logger/RealDefaultLogger.h"
#include "src/log/severity-macro-consts.h"

using namespace testing;
using namespace ImagesAnnotatorDataImporters014;
using namespace default_logger;

namespace
{

/**
 * @brief The application owned real logger substitution which records every
 * received message instead of the actual logging.
 */
class RecordingRealLogger : public RealDefaultLogger
{
 public:
  using RealDefaultLogger::log;

  void log(const unsigned short& loglvl, const std::string& msg) override
  {
    lvls.push_back(loglvl);
    msgs.push_back(msg);
  }

  std::vector<unsigned short> lvls;
  std::vector<std::string> msgs;
};

/**
 * @brief The application owned logger which implements the logger::ILogger
 * interface directly instead of deriving from the RealDefaultLogger, the very
 * same way an application built on top of another logging library hands its own
 * logger over to the library. It records every received message instead of the
 * actual logging.
 */
class RecordingLogger : public logger::ILogger
{
 public:
  inline static const std::string lvlRepr{"REC"};

  void log(const unsigned short& loglvl, const std::string& msg) override
  {
    lvls.push_back(loglvl);
    msgs.push_back(msg);
  }

  void log(const unsigned short& loglvl, const char* const filePath,
           const int& fileLine, const std::string& msg) override
  {
    log(loglvl,
        std::string{filePath} + ":" + std::to_string(fileLine) + " : " + msg);
  }

  void logfile([[maybe_unused]] const std::string& filepath) override {}

  void print([[maybe_unused]] const bool toPrintValue) override {}

  void level([[maybe_unused]] const unsigned short& nlvl) override {}

  const std::string& lvl_repr(
      [[maybe_unused]] const unsigned short& glvl) override
  {
    return lvlRepr;
  }

  void init([[maybe_unused]] const std::string& filepath,
            [[maybe_unused]] const unsigned short& nlvl,
            [[maybe_unused]] const bool toPrintValue) override
  {
  }

  std::vector<unsigned short> lvls;
  std::vector<std::string> msgs;
};

}  // namespace

/**
 * @brief Component test which checks that a real logger instance created and
 * owned by the library user is really used by the library code itself.
 *
 * The test links the produced shared library and drives it through the public
 * IADataImportersFacade only, so the logger crosses the very same boundary it
 * crosses inside a consuming application.
 */
class CTEST_LibraryRealLogger : public Test
{
 public:
  inline static const std::string app_log_file =
      CTEST_LibraryRealLogger_DATA_DIR "/CTEST_LibraryRealLogger.log";

  inline static const std::string no_context_msg =
      "No library context of a known dataset layout given";

  CTEST_LibraryRealLogger() { clear_log_file(); }

  ~CTEST_LibraryRealLogger()
  {
    // The instance adopted by the shared library can not be read back through
    // its public interface, so a fresh default logger is handed over instead
    // to keep the tests independent of each other.
    IADataImportersFacade::accept_real_logger(
        std::make_shared<RealDefaultLogger>());
    clear_log_file();
  }

  /**
   * @brief Asks the library for an importer without a context the very same
   * way its user does: the library reports the
   * CTEST_LibraryRealLogger::no_context_msg error and returns a nullptr.
   */
  IImporterPtr create_no_context_importer()
  {
    return IADataImportersFacade::create_importer({});
  }

  std::string log_contents()
  {
    std::fstream log{app_log_file.c_str(), std::fstream::in};

    std::string rt;
    char c{0};

    while (log.get(c)) {
      rt += c;
    }

    log.close();

    return rt;
  }

  void clear_log_file()
  {
    std::fstream log{app_log_file.c_str(),
                     std::fstream::out | std::fstream::trunc};

    log.close();
  }
};

TEST_F(CTEST_LibraryRealLogger, library_logs_reach_the_given_real_logger)
{
  const auto appLogger = std::make_shared<RecordingRealLogger>();

  IADataImportersFacade::accept_real_logger(appLogger);

  EXPECT_EQ(create_no_context_importer(), nullptr);

  EXPECT_THAT(appLogger->msgs, Contains(EndsWith(no_context_msg)));
  EXPECT_THAT(appLogger->lvls, Contains(logger::ILogger::LVL_ERROR));
}

TEST_F(CTEST_LibraryRealLogger, null_real_logger_leaves_the_previous_one)
{
  const auto appLogger = std::make_shared<RecordingRealLogger>();

  IADataImportersFacade::accept_real_logger(appLogger);
  IADataImportersFacade::accept_real_logger(nullptr);

  EXPECT_EQ(create_no_context_importer(), nullptr);

  EXPECT_THAT(appLogger->msgs, Contains(EndsWith(no_context_msg)));
}

TEST_F(CTEST_LibraryRealLogger, library_logs_reach_the_given_interface_logger)
{
  const auto appLogger = std::make_shared<RecordingLogger>();

  IADataImportersFacade::accept_real_logger(appLogger);

  EXPECT_EQ(create_no_context_importer(), nullptr);

  EXPECT_THAT(appLogger->msgs, Contains(EndsWith(no_context_msg)));
  EXPECT_THAT(appLogger->lvls, Contains(logger::ILogger::LVL_ERROR));
}

TEST_F(CTEST_LibraryRealLogger, library_logs_land_in_the_application_log_file)
{
  const auto appLogger = std::make_shared<RealDefaultLogger>();

  appLogger->init(app_log_file, MACRO_LVL_ERROR, false);

  IADataImportersFacade::accept_real_logger(appLogger);

  EXPECT_EQ(create_no_context_importer(), nullptr);

  EXPECT_THAT(log_contents(), HasSubstr(no_context_msg));
}
