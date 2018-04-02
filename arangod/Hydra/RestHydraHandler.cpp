////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2018 ArangoDB GmbH, Cologne, Germany
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
/// @author Simon Grätzer
////////////////////////////////////////////////////////////////////////////////

#include "RestHydraHandler.h"

#include "ApplicationFeatures/ApplicationServer.h"
#include "Cluster/ServerState.h"
#include "Hydra/Conductor.h"
#include "Hydra/JobContext.h"
#include "Hydra/HydraFeature.h"
#include "VocBase/ticks.h"

#include <velocypack/Builder.h>
#include <velocypack/velocypack-aliases.h>

using namespace arangodb;
using namespace arangodb::basics;
using namespace arangodb::rest;
using namespace arangodb::hydra;


RestHydraHandler::RestHydraHandler(GeneralRequest* request, GeneralResponse* response)
: RestVocbaseBaseHandler(request, response) {}

RestStatus RestHydraHandler::execute() {
  
  std::vector<std::string> const& suffix = _request->suffixes();
  if (suffix.size() != 2) {
    generateError(rest::ResponseCode::BAD,
                  TRI_ERROR_NOT_IMPLEMENTED, "unsupported operation");
    return RestStatus::DONE;
  }
  
  if (suffix[0] == "job") {
    handleJobControl();
  } if (suffix[0] == "notify") {
    handleNotify();
  } if (suffix[0] == "mailbox") {
    handleMailbox();
  } else {
    generateError(rest::ResponseCode::BAD,
                  TRI_ERROR_NOT_IMPLEMENTED, "unsupported operation");
  }
  
  return RestStatus::DONE;
}

void RestHydraHandler::handleJobControl() {
  if (_request->requestType() != rest::RequestType::POST) {
    generateError(rest::ResponseCode::METHOD_NOT_ALLOWED,
                  TRI_ERROR_NOT_IMPLEMENTED, "illegal method for /_api/hydra/job");
    return;
  }
  
  std::vector<std::string> const& suffix = _request->suffixes();
  TRI_ASSERT(suffix.size() == 2);
  hydra::HydraFeature* feature = hydra::HydraFeature::instance();
  TRI_ASSERT(feature != nullptr);
  JobId jid = _request->parsedValue("job", static_cast<ChannelId>(0));

  if (suffix[1] == "create") { // Public
    
    bool parseSuccess = true;
    VPackSlice body = parseVPackBody(parseSuccess);
    if (!parseSuccess || !body.isObject()) {
      // error message generated in parseVelocyPackBody
      return;
    }
    
    if (ServerState::instance()->isSingleServerOrCoordinator()) {
      jid = TRI_newServerTick();
#warning determine target servers and stuff like that on job context creation
    } else if (jid == 0) {
      generateError(rest::ResponseCode::BAD,
                    TRI_ERROR_BAD_PARAMETER, "invalid job id");
      return;
    }
  
    feature->addJob(std::make_unique<JobContext>(jid, body));
    
    JobContext* job = feature->job(jid);
    TRI_ASSERT(job);
    job->conductor()->startJob(); // trigger start job
    
  } else if (suffix[1] == "start") {  // internal
    
    JobContext* job = feature->job(jid);
    if (job == nullptr) {
      generateError(rest::ResponseCode::BAD,
                    TRI_ERROR_BAD_PARAMETER, "invalid job id");
      return;
    }
    job->conductor()->startJob();
    
  } else if (suffix[1] == "cancel") {  // public
    JobContext* job = feature->job(jid);
    if (job == nullptr) {
      generateError(rest::ResponseCode::BAD,
                    TRI_ERROR_BAD_PARAMETER, "invalid job id");
      return;
    }
    job->conductor()->cancelJob();
  }
  
  VPackBuilder builder;
  builder.openObject();
  //builder.add("algorithm", VPackValue(jid));
  builder.add("jobId", VPackValue(jid));
  builder.close();
  
  generateOk(rest::ResponseCode::OK, builder.slice());
}

void RestHydraHandler::handleNotify() {
  if (_request->requestType() != rest::RequestType::POST) {
    generateError(rest::ResponseCode::METHOD_NOT_ALLOWED,
                  TRI_ERROR_NOT_IMPLEMENTED, "illegal method for /_api/hydra/notify");
    return RestStatus::DONE;
  }
  
  bool parseSuccess = true;
  VPackSlice body = parseVPackBody(parseSuccess);
  if (!parseSuccess || !body.isObject()) {
    // error message generated in parseVelocyPackBody
    return;
  }
  
  std::vector<std::string> const& suffix = _request->suffixes();
  TRI_ASSERT(suffix.size() == 2);
  hydra::HydraFeature* feature = hydra::HydraFeature::instance();
  TRI_ASSERT(feature != nullptr);
  JobId jid = _request->parsedValue("job", static_cast<ChannelId>(0));
  
  JobContext* job = feature->job(jid);
  if (job == nullptr) {
    generateError(rest::ResponseCode::BAD,
                  TRI_ERROR_BAD_PARAMETER, "invalid job id");
    return;
  }
  
  if (!ServerState::instance()->isCoordinator()) {
    generateError(rest::ResponseCode::BAD,
                  TRI_ERROR_BAD_PARAMETER, "only supported on coordinator");
    return;
  }
  
  if (suffix[0] == "canceled") {
    std::string host = body.get("sender").copyString();
    job->conductor()->notifyCanceledJob(host);
    
    VPackBuilder builder;
    builder.openObject();
    builder.add("jobId", VPackValue(jid));
    builder.close();
    generateOk(rest::ResponseCode::OK, builder.slice());
  } else {
    generateError(rest::ResponseCode::BAD,
                  TRI_ERROR_BAD_PARAMETER, "unsupported path");
  }
}

void RestHydraHandler::handleMailbox() {
  if (_request->requestType() != rest::RequestType::PUT) {
    generateError(rest::ResponseCode::METHOD_NOT_ALLOWED,
                  TRI_ERROR_NOT_IMPLEMENTED, "illegal method for /_api/hydra/mailbox");
    return;
  }
  
  bool parseSuccess = true;
  VPackSlice body = parseVPackBody(parseSuccess);
  if (!parseSuccess || !body.isObject()) {
    // error message generated in parseVelocyPackBody
    return;
  }
  
  std::vector<std::string> const& suffix = _request->suffixes();
  TRI_ASSERT(suffix.size() == 2);
  hydra::HydraFeature* feature = hydra::HydraFeature::instance();
  TRI_ASSERT(feature != nullptr);
  
  JobId jid = _request->parsedValue("job", static_cast<ChannelId>(0));
  ChannelId chan = _request->parsedValue("channel", static_cast<ChannelId>(0));
  JobContext* job = feature->job(jid);
  if (job == nullptr) {
    generateError(rest::ResponseCode::BAD,
                  TRI_ERROR_BAD_PARAMETER, "invalid job id");
    return;
  }
  
  ChannelProgress prg = _request->parsedValue("progress", static_cast<ChannelProgress>(0));
  if (suffix[1] == "receive") {
    job->mailbox->receive(chan, prg, body);
  } else (suffix[1] == "done") {
    job->mailbox->receive(chan, prg);
  }
  
  VPackBuilder builder;
  builder.openObject();
  builder.add("jobId", VPackValue(jid));
  builder.close();
  generateOk(rest::ResponseCode::OK, builder.slice());
}
