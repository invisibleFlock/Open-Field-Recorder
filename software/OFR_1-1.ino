

#include <SDHCI.h>
#include <Audio.h>
#include <RTC.h>
#include <MP.h>  
#include <GNSS.h>

#define MY_TIMEZONE_IN_SECONDS (12 * 60 * 60)
#define TIME_HEADER 'T'

SDClass theSD;
AudioClass *theAudio;
SpGnss Gnss;
File myFile;


int noChar = 0;
int counter = 0;
String fileCounter ="";
char fileCounterChar = "";
int fileCounterInt = 0;
int fileCounterCharInt = 0;
bool updateTime = false;
bool ErrEnd = false;

// main variables for settings

bool gpsOn =  true;
bool audioQuality = false;
bool stereo = true;
bool sensor = true;
int gain = 160;


//this has been halved for stereo
static const int32_t recoding_frames_stereo = 18750;
// double for 4 chan
static const int32_t recoding_frames_4Chan = 37500;

static const int32_t recoding_size = recoding_frames_stereo * 6144; /* 4ch, 16bit, 768sample */

/**
 * @brief Audio attention callback
 *
 * When audio internal error occurc, this function will be called back.
 */

static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
  Serial.println("Attention!");

  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
  {
    ErrEnd = true;
  }
}


int setup_audio()
{
  err_t err;

  Serial.println("Setup Audio Library");

  theAudio = AudioClass::getInstance();

  err = theAudio->begin(audio_attention_cb);
  if (err != AUDIOLIB_ECODE_OK) {
    Serial.println("theAudio->begin error");
    return -1;
  }

// settings for audio quality 
if (audioQuality){
  //settings for 92000Hz sample rate
 err = theAudio->setRenderingClockMode(AS_CLKMODE_HIRES);
   err = theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC, 160, 327680);
      theAudio->initRecorder(AS_CODECTYPE_WAV, "/mnt/spif/BIN", AS_SAMPLINGRATE_192000, AS_CHANNEL_STEREO);
  if (err != AUDIOLIB_ECODE_OK) {
    Serial.println("theAudio->setRenderingClockMode error");
    return -1;
  }
  }
  else{
// settings for normal 48000
  err = theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC, 160, 327680);
      theAudio->initRecorder(AS_CODECTYPE_WAV, "/mnt/spif/BIN", AS_SAMPLINGRATE_48000, AS_CHANNEL_STEREO);

  if (err != AUDIOLIB_ECODE_OK) {
    Serial.println("theAudio->setRecorderMode error");
    return -1;
  }
  }

  sleep(1);

  /*
   * Initialize filetype to stereo wav with 48 Kb/s sampling rate
   * Search for WAVDEC codec in "/mnt/sd0/BIN" directory
   */


  return 0;
}

int start_record()
{
  err_t err;

if(gpsOn){
  GPS_update();
}
  if (Serial.available())
  {
    if (Serial.find(TIME_HEADER))
    {
      uint32_t pctime = Serial.parseInt();
      RtcTime rtc(pctime);
      RTC.setTime(rtc);
      Serial.println("timeupdated");
    }
  }

  if(fileCounterInt >= 1100){
    Serial.println("SD Card Full");
    MP.Send(1, 7, 1);
    while(1);
  }

  fileCounterInt = fileCounterInt + 1;
  theSD.remove("counter.txt");
  
  myFile = theSD.open("counter.txt", FILE_WRITE);
  if (!myFile)
  {
    Serial.println("File writing open error");
    return -1;
  }
  myFile.print(fileCounterInt);
  Serial.print("Written to file - ");
  Serial.println(fileCounterInt);
  myFile.close();
  
  RtcTime now = RTC.getTime();

  char filename[20] = {0};
  sprintf(filename, "M%02dD%02dH%02dM%02dS%02d.wav", now.month(), now.day(), now.hour(), now.minute(), now.second());
  Serial.println(filename);

  /* Open file for data write on SD card */
  myFile = theSD.open(filename, FILE_WRITE);
  if (!myFile)
  {
    Serial.println("File open error");
    MP.Send(1, 3, 1);
    return -1;
  }

  err = theAudio->writeWavHeader(myFile);
  if (err != AUDIOLIB_ECODE_OK) {
    Serial.println("theAudio->writeWavHeader error");
    myFile.close();
    return -2;
  }
  MP.Send(4, 1, 1);
  err = theAudio->startRecorder();
  if (err != AUDIOLIB_ECODE_OK) {
    Serial.println("theAudio->startRecorder error");
    myFile.close();
    return -3;
  }


  Serial.println("Recording Start!");

  return 0;
}

void stop_record()
{
  theAudio->stopRecorder();
  theAudio->closeOutputFile(myFile);
  myFile.close();
  counter++;

  Serial.println("End Recording");
  MP.Send(4, 2, 1);
}

void GPS_update()
{
  /* Wait for GNSS data */
  //Serial.println("Gnss.waitUpdate");
  if (Gnss.waitUpdate())
  {
    SpNavData NavData;

    /* Get the UTC time */
    Gnss.getNavData(&NavData);
    SpGnssTime *time = &NavData.time;

    /* Check if the acquired UTC time is accurate */
    if (time->year >= 2000)
    {
      RtcTime now = RTC.getTime();
      /* Convert SpGnssTime to RtcTime */
      RtcTime gps(time->year, time->month, time->day,
                  time->hour, time->minute, time->sec, time->usec * 1000);
      updateTime = true;
#ifdef MY_TIMEZONE_IN_SECONDS
      /* Set the time difference */
      gps += MY_TIMEZONE_IN_SECONDS;
#endif
      int diff = now - gps;
      if (abs(diff) >= 1)
      {
        RTC.setTime(gps);
        Serial.println("Gnss Updated");
      }
    }
  }
}

void setVar(){
  
  
  }

void setup()
{
  
  int ret;
  setVar();

  Serial.begin(115200);

  /* Wait HW initialization done. */
  sleep(3);
if(gpsOn){
  ret = Gnss.begin();
  if (ret != 0)
  {
    Serial.println("Gnss.begin error");
    assert(ret == 0);
  }

  ret = Gnss.start(COLD_START);
  if (ret != 0)
  {
    Serial.println("Gnss.start error");
    assert(ret == 0);
  }
}
  ret = MP.begin(1);
  if (ret < 0)
  {
    Serial.println("MP.begin error");
  }

  RTC.begin();

  ret = setup_audio();
  if (ret < 0)
  {
    Serial.println("setup_audio error");
    assert(ret == 0);
  }

  ledOn(LED3);
  if(gpsOn){
  Serial.println("Waiting for GPS");
  while (!updateTime)
  {
    GPS_update();
  }
  }

  ledOff(LED3);
  MP.Send(1, 6, 1);
    
    myFile = theSD.open("counter.txt", FILE_READ);
  if (!myFile)
  {
    Serial.println("File reading open error");
    return -1;
  }

  while (myFile.available()){
    fileCounterChar = myFile.read();
    fileCounter += fileCounterChar;
    fileCounterInt = 0;
  }
  noChar = fileCounter.length();
  if(noChar >= 1){
  for(int i=0; i <= (noChar-1) ; i++){
    fileCounterCharInt = (fileCounter[i] - '0');
    fileCounterCharInt = fileCounterCharInt*pow(10,(noChar - (i+1)));
    //Serial.print("Char - ");
    //Serial.println(fileCounterCharInt);
    fileCounterInt = fileCounterInt + fileCounterCharInt;
    //Serial.print("file Counter - ");
    //Serial.println(fileCounterInt);
  }
  }else{
    fileCounterInt = 0;
  }
    //fileCounterInt = fileCounter - '0';
    Serial.print("Read from file - ");
    Serial.println(fileCounterInt);

    myFile.close();
}

void loop()
{
  int ret;
  err_t err = AUDIOLIB_ECODE_OK;

  Serial.println("Start record!");

  ret = start_record();
  if (ret != 0) {
    Serial.print("start_record error = ");
    Serial.println(ret);
    sleep(1);
    return;
  }

  /* recording end condition */
  //Serial.println("Read and save record!");

  while (theAudio->getRecordingSize() < recoding_size && err == AUDIOLIB_ECODE_OK && !ErrEnd)
  {
    /* Read frames to record in file */
    err = theAudio->readFrames(myFile);
  }

  sleep(1);

  //Serial.println("Stop record!");

  stop_record();

  if (err != AUDIOLIB_ECODE_OK)
  {
    Serial.println("theAudio->readFrames error");
    MP.Send(1, 4, 1);
  }

  if (ErrEnd)
  {
    Serial.println("Error End");
    MP.Send(1, 5, 1);
    ErrEnd = false;
  }
}
