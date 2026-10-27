//Ranveer Gupta 
//2401mc20
//Assignment 9
//Q3

#include "types.h"
#include "stat.h"
#include "user.h"

// A9 - Q3:
// Page Replacement Algorithms (FIFO and LRU)
// Reference string: 7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1
// Expected for 3 frames:
//   FIFO: 15 page faults
//   LRU:  12 page faults

#define MAX_REF 100
#define MAX_FRAMES 10

// Print the current contents of the frames array
void
print_frames(int frames[], int num_frames)
{
  printf(1, "[");
  for(int i = 0; i < num_frames; i++){
    if(frames[i] == -1)
      printf(1, "  -");
    else {
      if(frames[i] < 10)
        printf(1, "  %d", frames[i]);
      else
        printf(1, " %d", frames[i]);
    }
  }
  printf(1, " ]");
}

// Simulate FIFO page replacement
// Returns number of page faults
int
simulate_fifo(int ref[], int n, int num_frames, int verbose)
{
  int frames[MAX_FRAMES];
  int arrival[MAX_FRAMES];
  int faults = 0;
  int hits = 0;
  int timer = 0;

  for(int i = 0; i < num_frames; i++){
    frames[i] = -1;
    arrival[i] = -1;
  }

  if(verbose){
    printf(1, "--------------------------------------------------------\n");
    printf(1, " Step | Ref | Frames                | Status\n");
    printf(1, "------+-----+-----------------------+---------\n");
  }

  for(int i = 0; i < n; i++){
    int page = ref[i];
    int hit = 0;
    timer++;

    // Check if page is already in a frame
    for(int f = 0; f < num_frames; f++){
      if(frames[f] == page){
        hit = 1;
        break;
      }
    }

    if(hit){
      hits++;
      if(verbose){
        if(i + 1 < 10)
          printf(1, "   %d  |  %d  | ", i + 1, page);
        else
          printf(1, "  %d  |  %d  | ", i + 1, page);
        print_frames(frames, num_frames);
        printf(1, " | Hit\n");
      }
    } else {
      faults++;
      // Check for an empty frame first
      int empty_idx = -1;
      for(int f = 0; f < num_frames; f++){
        if(frames[f] == -1){
          empty_idx = f;
          break;
        }
      }

      if(empty_idx != -1){
        frames[empty_idx] = page;
        arrival[empty_idx] = timer;
      } else {
        // Find the oldest frame (FIFO: minimum arrival time)
        int victim = 0;
        int oldest_time = arrival[0];
        for(int f = 1; f < num_frames; f++){
          if(arrival[f] < oldest_time){
            oldest_time = arrival[f];
            victim = f;
          }
        }
        frames[victim] = page;
        arrival[victim] = timer;
      }

      if(verbose){
        if(i + 1 < 10)
          printf(1, "   %d  |  %d  | ", i + 1, page);
        else
          printf(1, "  %d  |  %d  | ", i + 1, page);
        print_frames(frames, num_frames);
        printf(1, " | Fault\n");
      }
    }
  }

  if(verbose){
    printf(1, "--------------------------------------------------------\n");
    int ratio_t = (hits * 1000) / n;
    printf(1, "Total References: %d, Faults: %d, Hits: %d, Hit Ratio: %d.%d%%\n\n",
           n, faults, hits, ratio_t / 10, ratio_t % 10);
  }

  return faults;
}

// Simulate LRU page replacement
// Returns number of page faults
int
simulate_lru(int ref[], int n, int num_frames, int verbose)
{
  int frames[MAX_FRAMES];
  int last_used[MAX_FRAMES];
  int faults = 0;
  int hits = 0;
  int timer = 0;

  for(int i = 0; i < num_frames; i++){
    frames[i] = -1;
    last_used[i] = -1;
  }

  if(verbose){
    printf(1, "--------------------------------------------------------\n");
    printf(1, " Step | Ref | Frames                | Status\n");
    printf(1, "------+-----+-----------------------+---------\n");
  }

  for(int i = 0; i < n; i++){
    int page = ref[i];
    int hit_idx = -1;
    timer++;

    // Check if page is in frame
    for(int f = 0; f < num_frames; f++){
      if(frames[f] == page){
        hit_idx = f;
        break;
      }
    }

    if(hit_idx != -1){
      hits++;
      last_used[hit_idx] = timer; // update recency
      if(verbose){
        if(i + 1 < 10)
          printf(1, "   %d  |  %d  | ", i + 1, page);
        else
          printf(1, "  %d  |  %d  | ", i + 1, page);
        print_frames(frames, num_frames);
        printf(1, " | Hit\n");
      }
    } else {
      faults++;
      // Check for empty frame first
      int empty_idx = -1;
      for(int f = 0; f < num_frames; f++){
        if(frames[f] == -1){
          empty_idx = f;
          break;
        }
      }

      if(empty_idx != -1){
        frames[empty_idx] = page;
        last_used[empty_idx] = timer;
      } else {
        // Find the least recently used frame (minimum last_used time)
        int victim = 0;
        int min_used = last_used[0];
        for(int f = 1; f < num_frames; f++){
          if(last_used[f] < min_used){
            min_used = last_used[f];
            victim = f;
          }
        }
        frames[victim] = page;
        last_used[victim] = timer;
      }

      if(verbose){
        if(i + 1 < 10)
          printf(1, "   %d  |  %d  | ", i + 1, page);
        else
          printf(1, "  %d  |  %d  | ", i + 1, page);
        print_frames(frames, num_frames);
        printf(1, " | Fault\n");
      }
    }
  }

  if(verbose){
    printf(1, "--------------------------------------------------------\n");
    int ratio_t = (hits * 1000) / n;
    printf(1, "Total References: %d, Faults: %d, Hits: %d, Hit Ratio: %d.%d%%\n\n",
           n, faults, hits, ratio_t / 10, ratio_t % 10);
  }

  return faults;
}

int
main(int argc, char *argv[])
{
  int default_ref[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
  int default_len = sizeof(default_ref) / sizeof(default_ref[0]);

  int ref[MAX_REF];
  int ref_len;
  int user_frames = 0;

  if(argc > 2){
    user_frames = atoi(argv[1]);
    ref_len = argc - 2;
    if(ref_len > MAX_REF)
      ref_len = MAX_REF;
    for(int i = 0; i < ref_len; i++){
      ref[i] = atoi(argv[i + 2]);
    }
  } else {
    ref_len = default_len;
    for(int i = 0; i < default_len; i++){
      ref[i] = default_ref[i];
    }
    user_frames = 3;
  }

  printf(1, "========================================================\n");
  printf(1, "  pagerepl: Page Replacement Simulation (FIFO vs LRU)\n");
  printf(1, "========================================================\n\n");
  printf(1, "Reference String (%d items):\n  ", ref_len);
  for(int i = 0; i < ref_len; i++){
    printf(1, "%d ", ref[i]);
  }
  printf(1, "\n\n");

  // Task (b): Test with 3 frames and show detailed trace for FIFO and LRU
  printf(1, ">>> [Task b] Detailed FIFO Simulation (%d Frames) <<<\n", user_frames);
  int fifo_faults_3 = simulate_fifo(ref, ref_len, user_frames, 1);

  printf(1, ">>> [Task b] Detailed LRU Simulation (%d Frames) <<<\n", user_frames);
  int lru_faults_3 = simulate_lru(ref, ref_len, user_frames, 1);

  // Task (c): Run both algorithms for 3, 4, and 5 frames and print summary table
  printf(1, "========================================================================\n");
  printf(1, ">>> [Task c] Comparison Table for 3, 4, and 5 Frames <<<\n");
  printf(1, "========================================================================\n");
  printf(1, " Frames | FIFO Faults | FIFO Hit Ratio | LRU Faults | LRU Hit Ratio\n");
  printf(1, "--------+-------------+----------------+------------+---------------\n");

  for(int f = 3; f <= 5; f++){
    int f_faults = simulate_fifo(ref, ref_len, f, 0);
    int f_hits = ref_len - f_faults;
    int f_ratio = (f_hits * 1000) / ref_len;

    int l_faults = simulate_lru(ref, ref_len, f, 0);
    int l_hits = ref_len - l_faults;
    int l_ratio = (l_hits * 1000) / ref_len;

    printf(1, "   %d    |     %d%s      |     %d.%d%%     |     %d%s     |     %d.%d%%\n",
           f,
           f_faults, (f_faults < 10 ? " " : ""),
           f_ratio / 10, f_ratio % 10,
           l_faults, (l_faults < 10 ? " " : ""),
           l_ratio / 10, l_ratio % 10);
  }
  printf(1, "========================================================================\n");
  printf(1, "Expected results for 3 frames: FIFO = 15 faults, LRU = 12 faults.\n");
  if(fifo_faults_3 == 15 && lru_faults_3 == 12){
    printf(1, "VERIFICATION: Both FIFO (15) and LRU (12) matched expected results!\n");
  }
  printf(1, "========================================================================\n");

  exit();
}

