#include <iostream>
#include <cstdio>
#include <vector>
#include <queue>
#include <cmath>
#define TIME_TO_TICK 1e5
using namespace std;

int PROPAGATION_DELAY = 3;

struct Packet {
  int enter_tick;
  int exit_tick;
  int packet_size;
  Packet(int enter_tick, int packet_size) :
    enter_tick(enter_tick), exit_tick(-1), packet_size(packet_size){}
};

class Simulator {
public:
Simulator(int total_tick, double lambda, int packet_length, int transmission_rate, int buffer_size = -1)
  : total_tick(total_tick), lambda(lambda), packet_length(packet_length),
    transmission_rate(transmission_rate), buffer_size(buffer_size),
    packet_counter(0), current_tick(0), packet_loss_counter(0),
    cumulative_packet_in_queue(0), total_in_queue_time(0), next_packet_arival_time(calc_arrival_time()),
    total_delay(0) {
}

void start() {
  for(current_tick = 0; current_tick <= total_tick; current_tick++) {
    bool idel = true;
    if (arrival()) idel = false;
    if (departure()) idel = false;
    if (idel && packet_queue.empty()) idel_tick_counter++;
    cumulative_packet_in_queue += packet_queue.size();
  }
  generate_report();
}

bool arrival(){
  if(current_tick >= next_packet_arival_time){
    packet_counter++;
    if (buffer_size == -1 || packet_queue.size() <= buffer_size) {
      packet_queue.push(new Packet(current_tick, packet_length));
      next_packet_arival_time = current_tick + calc_arrival_time();
      return true;
    }
    else {
      packet_loss_counter++;
    }
  }
  return false;
}

bool departure(){
  if (packet_queue.empty()) return false;
  if(current_tick >= packet_queue.front()->exit_tick){
    total_in_queue_time += current_tick - packet_queue.front()->enter_tick;
    Packet *temp = packet_queue.front();
    total_delay += (temp->exit_tick - temp->enter_tick);

    packet_queue.pop();
    delete temp;

    return true;
  }
  return false;
}

bool start_transmission() {
  if(packet_queue.size() > 0) {
    packet_queue.front()->exit_tick = current_tick + PROPAGATION_DELAY +
      static_cast<int>(static_cast<double>(packet_queue.front()->packet_size) / static_cast<double>(transmission_rate) / 1e6 * TIME_TO_TICK);
    return true;
  }
  return false;
}

void drop_frame() {
  if(!packet_queue.empty()) {
    Packet *temp = packet_queue.front();
    packet_queue.pop();
    delete temp;
  }
}

int     total_delay;
int     current_tick;

private:
// 1 tick = 10us

// propagation delay is 3 ticks

double  lambda;
int     packet_length;
int     transmission_rate;
int     buffer_size;
int     total_tick;

int     idel_tick_counter;
int     packet_counter;
int     packet_loss_counter;

int     next_packet_arival_time;

long long     cumulative_packet_in_queue;

long long     total_in_queue_time;
queue<Packet*> packet_queue;

int calc_arrival_time(){
  double u = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));   //generate random number between 0...1
  int arrival_time = (int)((-1 / lambda)*log(1 - u) * 1000000);
  return arrival_time;
}

void generate_report() {
  cout << "Average number of packets in the buffer: " << static_cast<double>(cumulative_packet_in_queue) / static_cast<double>(total_tick) << endl;
  cout << "Average sojourn time: " << static_cast<double>(total_in_queue_time / TIME_TO_TICK) / static_cast<double>(packet_counter) <<  " second"  << endl;
  cout << "idle time: " << idel_tick_counter / TIME_TO_TICK << " seconds" << endl;
  cout << "packet loss: " << packet_loss_counter << endl;
}

};

/*
int main() {
  int tick, lambda, l, c, k;
  cin >> tick >> lambda >> l >> c >> k;
  Simulator simulator(tick, lambda, l, c, k);
  simulator.start();
  return 0;
}
*/
