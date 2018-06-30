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
    Timing() {
        Rtc = new RtcDS3231<TwoWire>(Wire);

        Rtc->Begin();
        if (!Rtc->IsDateTimeValid()) {
            Serial.println("RTC lost confidence in the DateTime!");
        }

        if (!Rtc->GetIsRunning()) {
            Serial.println("RTC was not actively running, starting now");
            Rtc->SetIsRunning(true);
        }
        Rtc->Enable32kHzPin(false);
        Rtc->SetSquareWavePin(DS3231SquareWavePin_ModeNone);

        NTP.begin("pool.ntp.org");
        NTP.setInterval(63);

        if (NTP.getLastNTPSync() == 0 && Rtc->IsDateTimeValid()) {
            Serial.println("NTP set by RTC");
            setTime(getRtcTimestamp());
        }
    }

    void dumpLastNTPSync() { Serial.println("LastNTPSync: " + String(NTP.getLastNTPSync())); }

    void updateRtc() {
        if (now() > 1529939906 && NTP.getLastNTPSync() != 0) { // 25 Jun 2018 20:18:26 CHEL
            compiled.InitWithEpoch32Time(now());
            Rtc->SetDateTime(compiled);
            Serial.println("RTC updated by now()");
        }
    }

    String getRtcDateTime() { return rtcToString(Rtc->GetDateTime()); }

    long getRtcTimestamp() { return Rtc->GetDateTime().Epoch64Time(); }
};

#endif