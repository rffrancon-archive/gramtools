#include "sdsl/suffix_arrays.hpp"
#include "sdsl/wavelet_trees.hpp"
#include <cassert>
#include "bwt_search.h"
#include <tuple>
#include <cstdint>

using namespace sdsl;

uint32_t get_location(csa_wt<wt_int<bit_vector,rank_support_v5<>>> csa,
		      uint64_t left, uint64_t right,
		      uint32_t num, bool last, 
		      std::vector<int>& allele, std::vector<int> mask_a)
{
  if (num%2==1) {
 	site=num;
        if (!last) allele.push_back(1);
      }
  else {
    if (right-left==1) {
      site=num-1;
      allele.push_back(mask_a[csa[left]]);
    }
    else {
      site=num-1;
      for (i=left;i<right;i++)
        allele.push_back(mask_a[csa[i]]);
    }
  }
  return (site);    
  

  //problems:
  //avoid mask_s
  //when have matches to the right of both odd numbers
  //when result is an interval, each element needs to have location saved
