#ifndef TIMING_H
#define TIMING_H

#include <NtpClientLib.h>
#include <RtcDS3231.h>
#include <Wire.h>

class Timing {
   private:
    RtcDS3231<TwoWire> *Rtc;
    RtcDateTime compiled;

    String rtcToString(const RtcDateTime &dt) {
        char datestring[20];
        sprintf(datestring, "%02u:%02u:%02u", dt.Hour(), dt.Minute(), dt.Second());
        return String(datestring);
    }

   public:
    boolean syncEventTriggered = false;  // True if a time even has been triggered
    NTPSyncEvent_t ntpEvent;             // Last triggered event

    Timing() {}
    void init() {
        NTP.begin("pool.ntp.org");
        NTP.setInterval(30);

        NTP.onNTPSyncEvent([&](NTPSyncEvent_t event) {
            // ntpEvent = event;
            syncEventTriggered = true;
            processSyncEvent(event);
        });

        // if (NTP.getLastNTPSync() == 0 && Rtc->IsDateTimeValid()) {
        //     Serial.println("NTP set by RTC");
        //     setTime(getRtcTimestamp());
        // }
    }

    void dumpNtp() {
        if (syncEventTriggered) {
            processSyncEvent(ntpEvent);
            syncEventTriggered = false;
        } else {
            Serial.println("syncEventTriggered false");
        }
    }

    void processSyncEvent(NTPSyncEvent_t ntpEvent) {
        if (ntpEvent) {
            Serial.print("Time Sync error: ");
            if (ntpEvent == noResponse)
                Serial.println("NTP server not reachable");
            else if (ntpEvent == invalidAddress)
                Serial.println("Invalid NTP server address");
        } else {
            Serial.print("Got NTP time: ");
            Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
        }
    }

    void dumpLastNTPSync() { Serial.println("LastNTPSync: " + String(NTP.getLastNTPSync())); }

    // void updateRtc() {
    //     if (now() > 1529939906 && NTP.getLastNTPSync() != 0) { // 25 Jun 2018 20:18:26 CHEL
    //         compiled.InitWithEpoch32Time(now());
    //         Rtc->SetDateTime(compiled);
    //         Serial.println("RTC updated by now()");
    //     }
    // }

    // String getRtcDateTime() { return rtcToString(Rtc->GetDateTime()); }

    // long getRtcTimestamp() { return Rtc->GetDateTime().Epoch64Time(); }
};

#endif