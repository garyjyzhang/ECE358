#include <vector>
#include <map>
#include <stdlib.h>
#include <time.h>
#include <iostream>

#include "simulate.cc"
using namespace std;

int counter = 0;
int counter2 = 0;
class Station {
public:
  Station(int total_tick, int A, int L, int W) : retrans_count(1), transmission_duration(0), transmitting(false),
  receiving_start(-1), receiving_end(-1), q(Simulator(total_tick, A, L, W)), wait_counter(0), packets_generated(0) {
    srand(time(0));
    wait_time = BEB(retrans_count);
  }

  void sync_on_tick(int tick) {
    q.current_tick = tick;
    if(wait_counter > 0) {
      wait_counter--;
      transmission_duration = 0;
    } else if(transmitting) {
      transmission_duration++;
    }

    if(q.arrival()) {
      packets_generated++;
    }

    if(wait_counter == 0 && !transmitting) {
      // start transmission if there's a packet in the queue
      transmitting = q.start_transmission();
    }
  }

  bool is_transmitting() {
    return transmitting;
  }

  bool transmission_complete() {
    if(q.departure()) {
      transmitting = false;
      transmission_duration = 0;
      retrans_count = 1;
      return true;
    }
    return false;
  }

  bool is_receiving() {
    return q.current_tick >= receiving_start && (q.current_tick <= receiving_end || receiving_end < receiving_start);
  }


  void start_receiving_after_delay(int current_tick) {
    receiving_start = current_tick + PROPAGATION_DELAY;
  }

  void stop_receiving_after_delay(int current_tick) {
    receiving_end = current_tick + PROPAGATION_DELAY;
  }

  void wait() {
    transmitting = false;
    transmission_duration = 0;
    wait_counter = wait_time;
  }

  void wait_random() {
    // wait_time = BEB(retrans_count);
    wait_time = 0;
    wait();
  }

  void abort() {
    if(++retrans_count > 10) {
      q.drop_frame();
      retrans_count = 1;
    }
    wait_time = BEB(retrans_count);
    wait();
  }

  bool waiting() {
    return wait_counter > 0;
  }

  int get_transmission_duration() {
    return transmission_duration;
  }

  int get_total_delay() {
    return q.total_delay;
  }

  Simulator q;
  int packets_generated;
  int receiving_start, receiving_end;
  int wait_counter;
  int transmission_duration;

private:
  int BEB(int i) {
    return rand() % (int) (pow((double) 2, (double) i)) * PROPAGATION_DELAY;
  }

  bool transmitting;
  // one queue from p1
  int retrans_count;

  // indicates when the latest receiving of a frame starts and ends
  //the wait time for BEB
  int wait_time;
  // the amount of time in ticks that the latest transmission has lasted
};

class CSMA_CD {
public:
  CSMA_CD(int N, int A, int L, int W, int total_tick) :
  total_tick(total_tick), num_stations(N), packets_sent(0), packets_generated(0) {
    for(int i = 0; i < N; i++) {
      Station station(total_tick, A, L, W);
      stations.push_back(station);
    }
    srand(time(NULL));
  }
  void simulate() {
    for(int tick = 0; tick < total_tick; tick++) {
      for(int i = num_stations - 1; i >= 0; i--) {

        Station &station = stations[i];
        station.sync_on_tick(tick);

        // if the station is waiting or has nothing to transmit, simply move on to the next station
        if(station.waiting() || !station.is_transmitting()) continue;

        if(station.is_transmitting() && station.is_receiving()) {
          //cout << "collision: " << tick << endl;
          // stations attempts to start transmission but medium is busy
          if(station.get_transmission_duration() == 0) {
            counter2++;
            station.wait_random();
          }
          // station is already transmitting and sensed collision
          else {
            counter++;
            station.abort();
            for(int j = 0; j < num_stations; j++) {
              if(i != j)
                stations[j].stop_receiving_after_delay(tick);
            }
          }
          continue;
        }


        // station is transmitting and no collision has been detected
        if(station.get_transmission_duration() == 0) {
          for(int j = 0; j < num_stations; j++) {
            if(i != j)
              stations[j].start_receiving_after_delay(tick);
          }
        } else if(station.transmission_complete()) {
            for(int j = 0; j < num_stations; j++) {
              if(i != j)
                stations[j].stop_receiving_after_delay(tick);
            }
            packets_sent++;
        }
      }
    }


    long long total_delay = 0;
    for(int i = 0; i < num_stations; i++) {
      total_delay += stations[i].get_total_delay();
      packets_generated += stations[i].packets_generated;
    }

    cout << "packets generated: " << packets_generated << endl;
    cout << "packets sent: " << packets_sent << endl;
    cout << "Throughput: " << (double) packets_sent / ((double) total_tick / 1e5) << endl;
    cout << "Average delay: " << total_delay / (double) packets_sent / 100 << "ms" << endl;
  }

private:

  int packets_sent;
  int packets_generated;
  int total_tick;
  int num_stations;
  vector<Station> stations;
};


int main() {
  int N, A, total_tick;
  cin >> N >> A >> total_tick;
  CSMA_CD csma(N, A, 8000, 1, total_tick);
  csma.simulate();
  return 0;
}