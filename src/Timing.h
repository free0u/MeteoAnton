#ifndef TIMING_H
#define TIMING_H

class Timing {
   private:
    MeteoLog *meteoLog;

   public:
    Timing() {}
    void init(MeteoLog *meteoLog) {}
};

// #include <NtpClientLib.h>
// #include <RtcDS3231.h>
// #include <Wire.h>

// class Timing {
//    private:
//     RtcDS3231<TwoWire> *Rtc;
//     RtcDateTime compiled;
//     MeteoLog *meteoLog;
//     String rtcToString(const RtcDateTime &dt) {
//         char datestring[20];
//         sprintf(datestring, "%02u:%02u:%02u", dt.Hour(), dt.Minute(), dt.Second());
//         return String(datestring);
//     }

//    public:
//     boolean syncEventTriggered = false;  // True if a time even has been triggered
//     NTPSyncEvent_t ntpEvent;             // Last triggered event

//     Timing() {}
//     void init(MeteoLog *meteoLog) {
//         this->meteoLog = meteoLog;

//         meteoLog->add("NTP begin");
//         Serial.println("NTP begin");

//         meteoLog->add(String(now()));

//         NTP.begin("pool.ntp.org");
//         NTP.setInterval(25);

//         meteoLog->add(String(now()));

//         NTP.onNTPSyncEvent([&](NTPSyncEvent_t event) {
//             // meteoLog->add("NTP event");
//             // ntpEvent = event;
//             syncEventTriggered = true;
//             // processSyncEvent(event);
//         });

//         // if (NTP.getLastNTPSync() == 0 && Rtc->IsDateTimeValid()) {
//         //     Serial.println("NTP set by RTC");
//         //     setTime(getRtcTimestamp());
//         // }
//     }

//     // void dumpNtp() {
//     //     if (syncEventTriggered) {
//     //         processSyncEvent(ntpEvent);
//     //         syncEventTriggered = false;
//     //     } else {
//     //         meteoLog->add("syncEventTriggered false");
//     //     }
//     // }

//     void processSyncEvent(NTPSyncEvent_t ntpEvent) {
//         if (ntpEvent) {
//             meteoLog->add("Time Sync error: ");
//             if (ntpEvent == noResponse)
//                 meteoLog->add("NTP server not reachable");
//             else if (ntpEvent == invalidAddress)
//                 meteoLog->add("Invalid NTP server address");
//         } else {
//             meteoLog->add("Got NTP time: ");
//             meteoLog->add(NTP.getTimeDateString(NTP.getLastNTPSync()));
//         }
//     }

//     void dumpLastNTPSync() { Serial.println("LastNTPSync: " + String(NTP.getLastNTPSync())); }

//     // void updateRtc() {
//     //     if (now() > 1529939906 && NTP.getLastNTPSync() != 0) { // 25 Jun 2018 20:18:26 CHEL
//     //         compiled.InitWithEpoch32Time(now());
//     //         Rtc->SetDateTime(compiled);
//     //         Serial.println("RTC updated by now()");
//     //     }
//     // }

//     // String getRtcDateTime() { return rtcToString(Rtc->GetDateTime()); }

//     // long getRtcTimestamp() { return Rtc->GetDateTime().Epoch64Time(); }
// };

#endif