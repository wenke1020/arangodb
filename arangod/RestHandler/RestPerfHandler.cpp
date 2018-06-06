////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2018-2018 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Max Neunhoeffer
////////////////////////////////////////////////////////////////////////////////

#include "RestPerfHandler.h"
#include "ApplicationFeatures/ApplicationServer.h"
#include "Logger/Logger.h"
#include "Rest/HttpRequest.h"
#include "Scheduler/SchedulerFeature.h"
#include "Scheduler/Scheduler.h"

#include <velocypack/Builder.h>
#include <velocypack/velocypack-aliases.h>

#include <thread>
#include <mutex>
#include <condition_variable>

using namespace arangodb;

////////////////////////////////////////////////////////////////////////////////
/// @brief ArangoDB server
////////////////////////////////////////////////////////////////////////////////

RestPerfHandler::RestPerfHandler(GeneralRequest* request,
                                 GeneralResponse* response)
    : RestBaseHandler(request, response) {}

bool RestPerfHandler::isDirect() const { return false; }

int64_t avoidOptimization = 0;

namespace {
  struct Times {
    std::chrono::high_resolution_clock::time_point post;
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
  };

  double timeDiff(std::chrono::high_resolution_clock::time_point& a,
                  std::chrono::high_resolution_clock::time_point& b) {
    return std::chrono::duration_cast<
             std::chrono::duration<double>>(b-a).count();
  }

  int64_t calibrate(double seconds) {
    // This will calibrate a delay loop and return a number to which to
    // count to create a busy worker of `seconds` seconds.
    auto runner = [](int64_t n, int64_t i) -> int64_t {
      int64_t x = 0;
      int64_t ii = i;
      for (int64_t j = 0; j < n; j++) {
        x += ii;
        ii += 3;
      }
      return x;
    };

    LOG_TOPIC(INFO, Logger::FIXME) << "Calibrating for time " << seconds;
    int64_t n = 500;
    double t = 0.0;
    int64_t i = 1;
    // First start with counting to n and double until it took longer
    // than seconds:
    while (t < seconds) {
      n *= 2;
      auto startTime = std::chrono::high_resolution_clock::now();
      avoidOptimization += runner(n, ++i);
      auto endTime = std::chrono::high_resolution_clock::now();
      t = timeDiff(startTime, endTime);
    }
    // Now search for the right value:
    int64_t sum = 0;
    for (int j = 0; j < 20; ++j) {
      n = static_cast<int64_t>(n * (seconds / t));
      auto startTime = std::chrono::high_resolution_clock::now();
      avoidOptimization += runner(n, ++i);
      auto endTime = std::chrono::high_resolution_clock::now();
      t = timeDiff(startTime, endTime);
      sum += n;
    }
    n = sum/20;
    double sumd = 0;
    for (int j = 0; j < 20; ++j) {
      auto startTime = std::chrono::high_resolution_clock::now();
      avoidOptimization += runner(n, ++i);
      auto endTime = std::chrono::high_resolution_clock::now();
      t = timeDiff(startTime, endTime);
      LOG_TOPIC(INFO, Logger::FIXME) << "Cal: n=" << n << " t=" << t;
      sumd += t;
    }
    
    LOG_TOPIC(INFO, Logger::FIXME) << "Calibration result: n=" << n
      << ", t(avg)=" << sumd/20;
    return n;
  }

  std::pair<int64_t, double> dowork0(int64_t number, int64_t complexity,
                  std::vector<Times>& times) {
    auto start = std::chrono::high_resolution_clock::now();
    std::condition_variable condvar;
    std::mutex mutex;
    auto sched 
      = arangodb::application_features::ApplicationServer::lookupFeature<
          arangodb::SchedulerFeature>("Scheduler")->SCHEDULER;
    std::atomic<int64_t> accumulator(0);
    std::atomic<int64_t> counter(0);
    for (int64_t i = 0; i < number; ++i) {
      times[i].post = std::chrono::high_resolution_clock::now();
      sched->post([&accumulator, &counter, &number, &condvar, &times, complexity, i]() {
                    times[i].start = std::chrono::high_resolution_clock::now();
                    int64_t x = 0;
                    int64_t ii = i;
                    for (int64_t j = 0; j < complexity; j++) {
                      x += ii;
                      ii += 3;
                    }
                    accumulator += x;
                    times[i].end = std::chrono::high_resolution_clock::now();
                    if (++counter == number) {
                      condvar.notify_one();
                    }
                  });
    }
    auto end = std::chrono::high_resolution_clock::now();
    LOG_TOPIC(INFO, Logger::FIXME) << "Time for posting: "
      << timeDiff(start, end);
    std::unique_lock<std::mutex> lock(mutex);
    condvar.wait(lock, [&counter, &number]() {
                         return counter.load() == number;
                       });
    return std::make_pair(accumulator.load(), timeDiff(start, end));
  }

  std::pair<int64_t, double> dowork1(int64_t number, int64_t complexity,
                  int64_t inFlight, std::vector<Times>& times) {
    auto start = std::chrono::high_resolution_clock::now();
    std::condition_variable condvar;
    std::mutex mutex;
    auto sched 
      = arangodb::application_features::ApplicationServer::lookupFeature<
          arangodb::SchedulerFeature>("Scheduler")->SCHEDULER;
    std::atomic<int64_t> accumulator(0);
    std::atomic<int64_t> counter(0);
    std::atomic<int64_t> currentlyInFlight(0);
    for (int64_t i = 0; i < number; ++i) {
      while (currentlyInFlight.load(std::memory_order_relaxed) > inFlight) {
        usleep(1);
      }
      ++currentlyInFlight;
      times[i].post = std::chrono::high_resolution_clock::now();
      sched->post([&accumulator, &counter, &number, &condvar, &times,
                   &currentlyInFlight, complexity, i]() {
                    times[i].start = std::chrono::high_resolution_clock::now();
                    int64_t x = 0;
                    int64_t ii = i;
                    for (int64_t j = 0; j < complexity; j++) {
                      x += ii;
                      ii += 3;
                    }
                    accumulator += x;
                    times[i].end = std::chrono::high_resolution_clock::now();
                    --currentlyInFlight;
                    if (++counter == number) {
                      condvar.notify_one();
                    }
                  });
    }
    auto end = std::chrono::high_resolution_clock::now();
    LOG_TOPIC(INFO, Logger::FIXME) << "Time for posting: "
      << timeDiff(start, end);
    std::unique_lock<std::mutex> lock(mutex);
    condvar.wait(lock, [&counter, &number]() {
                         return counter.load() == number;
                       });
    return std::make_pair(accumulator.load(), timeDiff(start, end));
  }

  std::pair<int64_t, double> dowork2(int64_t number, double time,
                                     std::vector<Times>& times) {
    // Calibration:
    auto start = std::chrono::high_resolution_clock::now();
    for (int64_t i = 0; i < number; ++i) {
      times[i].post = std::chrono::high_resolution_clock::now();
      times[i].start = std::chrono::high_resolution_clock::now();
      int64_t complexity = ::calibrate(time);
      times[i].end = std::chrono::high_resolution_clock::now();
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::make_pair(12, timeDiff(start, end));
  }
}

RestStatus RestPerfHandler::execute() {
  bool found;
  std::string buf;

  int64_t number = 1000;
  buf = _request->value("number", found);
  if (found) {
    number = arangodb::basics::StringUtils::int64(buf);
  }

  double time = 0.001;  // 1 ms
  buf = _request->value("time", found);
  if (found) {
    time = arangodb::basics::StringUtils::doubleDecimal(buf);
  }

  int64_t complexity = 1000000;
  buf = _request->value("complexity", found);
  if (found) {
    complexity = arangodb::basics::StringUtils::int64(buf);
  }

  int32_t type = 0;
  buf = _request->value("type", found);
  if (found) {
    type = arangodb::basics::StringUtils::int32(buf);
  }

  int64_t inFlight = 1000;
  buf = _request->value("inFlight", found);
  if (found) {
    inFlight = arangodb::basics::StringUtils::int64(buf);
  }

  int64_t result;
  double schedTime;
  std::vector<Times> times;
  times.resize(number);
  std::string typeSt;
  auto startTime = std::chrono::high_resolution_clock::now();
  switch (type) {
    case 0: {
      auto rr = ::dowork0(number, complexity, times);
      result = rr.first;
      schedTime = rr.second;
      typeSt = "Schedule all right away";
      break;
    }
    case 1: {
      auto rr = ::dowork1(number, complexity, inFlight, times);
      result = rr.first;
      schedTime = rr.second;
      typeSt = "Schedule constant number in flight";
      break;
    }
    case 2: {
      auto rr = ::dowork2(number, time, times);
      result = rr.first;
      schedTime = rr.second;
      typeSt = "Calibration";
      break;
    }
    default: {
      result = 0;
      schedTime = 0;
      break;
    }
  }
  auto endTime = std::chrono::high_resolution_clock::now();
  double dur = ::timeDiff(startTime, endTime);
  std::vector<double> diffs;
  diffs.resize(times.size());

  auto addStats = [](VPackBuilder& r, std::vector<double> diffs) {
    std::sort(diffs.begin(), diffs.end());
    { VPackObjectBuilder guard(&r);
      r.add("p50", VPackValue(diffs[(diffs.size()*50)/100]));
      r.add("p90", VPackValue(diffs[(diffs.size()*90)/100]));
      r.add("p95", VPackValue(diffs[(diffs.size()*95)/100]));
      r.add("p99", VPackValue(diffs[(diffs.size()*99)/100]));
      if (diffs.size() >= 20) {
        r.add(VPackValue("smallest10"));
        { VPackArrayBuilder guard2(&r);
          for (size_t i = 0; i < 10; ++i) {
            r.add(VPackValue(diffs[i]));
          }
        }
        r.add(VPackValue("largest10"));
        { VPackArrayBuilder guard2(&r);
          for (size_t i = 10; i > 0; --i) {
            r.add(VPackValue(diffs[diffs.size() - i]));
          }
        }
      }
    }
  };

  VPackBuilder r;
  { VPackObjectBuilder guard(&r);
    r.add("number", VPackValue(number));
    r.add("result", VPackValue(result));
    r.add("schedTime", VPackValue(schedTime));
    r.add("totalTime", VPackValue(dur));
    r.add("type", VPackValue(typeSt));
    if (type == 1) {
      r.add("inFlight", VPackValue(inFlight));
    }

    for (size_t i = 0; i < times.size(); ++i) {
      diffs[i] = timeDiff(times[i].start, times[i].end);
    }
    r.add(VPackValue("taskTimes"));
    addStats(r, diffs);

    for (size_t i = 0; i < times.size(); ++i) {
      diffs[i] = timeDiff(times[i].post, times[i].start);
    }
    r.add(VPackValue("waitTimes"));
    addStats(r, diffs);

    for (size_t i = 0; i < times.size(); ++i) {
      diffs[i] = timeDiff(times[i].post, times[i].end);
    }
    r.add(VPackValue("totalTimes"));
    addStats(r, diffs);
  }
  generateResult(rest::ResponseCode::OK, r.slice());
  return RestStatus::DONE;
}
