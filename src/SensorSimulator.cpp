
#include "SensorSimulator.h"
#include <cmath>
#include <cstdlib> // used to generate random numbers using rand
#include <algorithm>


static float clamp(float v, float lo, float hi) // used to clamp numbers that exceeds hi ( high ) or ( low )
{
return std::max(lo, std::min(hi, v));
}


SensorSimulator::SensorSimulator() 
{ 
  std::srand(42); // set the seeds 42 so everytime when we run the program it gives us different numbers
}


float SensorSimulator::noise(float amp) 
{
return ((float)std::rand() / RAND_MAX - 0.5f) * 2.0f * amp; // adding some fake noise
}


bool SensorSimulator::should_spike() 
{
if (spike_cooldown_ > 0) // so the spike become rare and not just spamming
{ 
  --spike_cooldown_; 
  return false;
}
 //making the percentage of having a spike is 3%
if ((std::rand() % 100) < 3){
spike_cooldown_ = 20; // if the spike happened , it won't happen again atleast after 20 attempts
return true;
}
return false;
}


void SensorSimulator::update(SensorData& d) 
{
bool spike = should_spike();
d.temperature = clamp(
d.temperature + noise(0.3f) + (spike ? noise(8.0f) : 0.0f), -10.0f, 80.0f); // if noise small change, if spike big change , the clamp function let the data vary between -amp & +amp

d.humidity = clamp(
d.humidity + noise(0.4f) + (spike ? noise(10.0f) : 0.0f), 0.0f, 100.0f); // same here 

d.distance = clamp(
d.distance + noise(2.0f) + (spike ? noise(40.0f) : 0.0f), 0.0f, 400.0f); // for distance i made the change a bit to much since the ultrasonic sensor isn't that stable

d.imu.x = clamp(d.imu.x + noise(0.05f), -1.0f, 1.0f); 
d.imu.y = clamp(d.imu.y + noise(0.05f), -1.0f, 1.0f);
d.imu.z = clamp(d.imu.z + noise(0.02f), -1.5f, 1.5f);

// update history, if the number exceeds 80 ( history length ) 
auto push = [](std::deque<float>& q, float v) 
{
q.push_back(v);
if (q.size() > SensorData::HISTORY_LEN) q.pop_front();
};

push(d.temp_history, d.temperature);
push(d.hum_history,  d.humidity);
push(d.dist_history, d.distance);

// checking status 
d.temp_status   = (d.temperature < 15 || d.temperature > 45) ? SensorStatus::WARN : SensorStatus::OK;
d.hum_status    = (d.humidity < 20 || d.humidity > 80) ? SensorStatus::WARN : SensorStatus::OK;
d.dist_status   = (d.distance < 5 || d.distance > 200) ? SensorStatus::WARN : SensorStatus::OK;
}
