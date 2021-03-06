/* Any copyright is dedicated to the Public Domain.
   http://creativecommons.org/publicdomain/zero/1.0/ */
"use strict";

/**
 * Tests if the profiler is populated by in-progress console recordings
 * when it is opened.
 */

const { Constants } = require("devtools/client/performance/modules/constants");
const { SIMPLE_URL } = require("devtools/client/performance/test/helpers/urls");
const { initPerformanceInTab, initConsoleInNewTab, teardownToolboxAndRemoveTab } = require("devtools/client/performance/test/helpers/panel-utils");
const { waitForRecordingStoppedEvents } = require("devtools/client/performance/test/helpers/actions");
const { waitUntil } = require("devtools/client/performance/test/helpers/wait-utils");
const { times } = require("devtools/client/performance/test/helpers/event-utils");

add_task(function* () {
  let { target, console } = yield initConsoleInNewTab({
    url: SIMPLE_URL,
    win: window
  });

  yield console.profile("rust");
  yield console.profile("rust2");

  let { panel } = yield initPerformanceInTab({ tab: target.tab });
  let { EVENTS, PerformanceController, OverviewView, RecordingsView } = panel.panelWin;

  yield waitUntil(() => PerformanceController.getRecordings().length == 2);

  let recordings = PerformanceController.getRecordings();
  is(recordings.length, 2, "Two recordings found in the performance panel.");
  is(recordings[0].isConsole(), true, "Recording came from console.profile (1).");
  is(recordings[0].getLabel(), "rust", "Correct label in the recording model (1).");
  is(recordings[0].isRecording(), true, "Recording is still recording (1).");
  is(recordings[1].isConsole(), true, "Recording came from console.profile (2).");
  is(recordings[1].getLabel(), "rust2", "Correct label in the recording model (2).");
  is(recordings[1].isRecording(), true, "Recording is still recording (2).");

  is(RecordingsView.selectedItem.attachment, recordings[0],
    "The first console recording should be selected.");
  is(RecordingsView.selectedItem.attachment.getLabel(), "rust",
    "The profile label for the first recording is correct.");

  // Ensure overview is still rendering.
  yield times(OverviewView, EVENTS.UI_OVERVIEW_RENDERED, 3, {
    expectedArgs: { "1": Constants.FRAMERATE_GRAPH_LOW_RES_INTERVAL }
  });

  let stopped = waitForRecordingStoppedEvents(panel, {
    // only emitted for manual recordings
    skipWaitingForBackendReady: true
  });
  yield console.profileEnd("rust");
  yield stopped;

  stopped = waitForRecordingStoppedEvents(panel, {
    // only emitted for manual recordings
    skipWaitingForBackendReady: true,
    // only emitted when a finished recording is selected
    skipWaitingForOverview: true,
    skipWaitingForSubview: true,
  });
  yield console.profileEnd("rust2");
  yield stopped;

  yield teardownToolboxAndRemoveTab(panel);
});
