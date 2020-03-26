#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>
#include <numeric>

void pi_thread_worker(const uint& nbpoint, const uint tid, const uint nbthread, std::vector<double>& output){
  double s = 0.;
  double l = 1./nbpoint;
  int start = tid*(nbpoint/nbthread);
  int stop = (tid+1)*(nbpoint/nbthread);
  if( tid == nbthread-1){
    stop += nbpoint%nbthread;
  }

  double x;
  for( int i=start; i<stop; i++){
    x = l * ( i + 0.5 );
    s += l * ( 4. / (1 + x*x ) );
  }
  output[tid] = s;
}


int main(int argc, char** argv ){
  if( argc == 1 ){
    std::cerr << "Specify the number of thread" << std::endl;
    return 1;
  }

  int nb_point = 100000000;
  int nb_thread = atoi(argv[1]);

  std::vector<double> pi_contrib(nb_thread, 0.);
  std::vector<std::thread> threads;
  for( int i=0; i<nb_thread; i++){
    threads.push_back( std::thread( pi_thread_worker, nb_point, i, nb_thread, std::ref(pi_contrib) ) );
  }

  for( int i=0; i<nb_thread; i++){
    threads[i].join();
  }

  double pi = std::accumulate( pi_contrib.begin(), pi_contrib.end(), 0.);
  if( argc > 2 ){
    std::cout << "pi approx = " << pi << std::endl;
  }
  

}
