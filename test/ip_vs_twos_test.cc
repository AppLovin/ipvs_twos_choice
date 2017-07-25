#include <gtest/gtest.h>
#include <iostream>
#include <vector>
#include <linux/random.h>

static int ip_vs_twos_schedule(std::vector<int> destinations) {
  int dest, choice1 = -1, choice2 = -1;
  int rweight1, rweight2, weight1 = 0, weight2 = 0, total_weight = 0,
                          weight = 0;

  /*
   * Generate a random weight between [0,sum of all weights)
   */
  for (dest = 0; dest < destinations.size(); ++dest) {
    weight = destinations[dest];
    if (weight > 0) {
      total_weight += weight;
      choice1 = dest;
    }
  }

  if (choice1 == -1) {
    std::cerr << "no destination available\n";
    return -1;
  }

  rweight1 = rand() % total_weight;
  rweight2 = rand() % total_weight;

  /*
   * Find the first weighted dest
   */
  for (dest = 0; dest < destinations.size(); ++dest) {
    weight = destinations[dest];
    if (weight > 0) {
      rweight1 -= weight;
      rweight2 -= weight;

      if (rweight1 <= 0 && choice1 != -1) {
        choice1 = dest;
        weight1 = weight;
      }

      if (rweight2 <= 0 && choice2 == -1) {
        choice2 = dest;
        weight2 = weight;
      }

      if (choice1 != -1 && choice2 != -1)
      {
        goto choicestage;
      }
    }
  }

choicestage:
  if (choice2 != -1 && weight2 > weight1) {
    choice1 = choice2;
    weight1 = weight2;
  }

  return choice1;
}

TEST(IPVSTwosTest, TestTwosChoice) {
    std::vector<int> dests = {10,10,10,10,10,10,10};

    for (int i = 0; i < 10000; ++i)
    {
        int choice = ip_vs_twos_schedule(dests);

        std::cout << "x " << choice << "\n";
    }
}

int main(int ac, char *av[]) {
  srand(time(NULL));
  testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}

