#include "sdsl/suffix_arrays.hpp"
#include "sdsl/wavelet_trees.hpp"
#include <cassert>
#include "bwt_search.h"
#include <tuple>
#include <cstdint>
#include "sitemarker.hpp"
#include "bidir_search_bwd.hpp"

using namespace sdsl;

// should make csa template to have control from cmd line over SA sampling density
std::vector<uint8_t>::iterator bidir_search_bwd(csa_wt<wt_int<bit_vector,rank_support_v5<>>,2,2> csa,
						uint64_t left, uint64_t right,
						uint64_t left_rev, uint64_t right_rev,
						std::vector<uint8_t>::iterator pat_begin, 
						std::vector<uint8_t>::iterator pat_end,
						interval_list& sa_intervals, 
						interval_list& sa_intervals_rev,
						SiteOverlapTracker* tracker,
						SiteMarkerArray const * const  prg_sites,
						std::vector<int> mask_a, uint64_t maxx, bool& first_del
						)
{

 
  // zam removing  std::list<std::vector<std::pair<uint32_t, std::vector<int>>>>::iterator it_s,it_s_end;

  std::vector<uint8_t>::iterator pat_it=pat_end;

  interval_list::iterator it, it_rev, it_end, it_rev_end;
  //  std::list<std::pair<uint64_t,uint64_t>>::iterator it, it_rev,it_end,it_rev_end;



  uint8_t c;
  bool last,ignore;
  uint64_t left_new, right_new, left_rev_new, right_rev_new;
  std::vector<std::pair<uint32_t, std::vector<int>>> empty_pair_vector;
  std::vector<std::pair<uint64_t,uint64_t>> res;   
  uint64_t init_list_size,j;

  assert(left<right);
  assert(right<=csa.size());

  if (sa_intervals.empty()) {
    sa_intervals.push_back(std::make_pair(left,right));
    sa_intervals_rev.push_back(std::make_pair(left_rev,right_rev));
  }
  int k=0;
  while (pat_it>pat_begin && !sa_intervals.empty()) {
    --pat_it;
    c=*pat_it;
    // k++;
    //if (sa_intervals.size()==0) cout<<k<<" "<<unsigned(c)<<endl;

    assert(sa_intervals.size()==sa_intervals_rev.size());
    assert(sa_intervals.size()==sites.size());//each interval has a corresponding vector of sites/alleles crossed; what about the first interval? (corresp to matches in the ref)

    it=sa_intervals.begin();
    it_rev=sa_intervals_rev.begin();
    //it_s=sites.begin();

    //it_end=sa_intervals.end(); // make these constant iterators
    //it_rev_end=sa_intervals_rev.end();
    //it_s_end=sites.end();
    init_list_size=sa_intervals.size();
    j=0;

    if (pat_it!=pat_end-1) {
      while(j<init_list_size) {
	//don't do this for first letter searched
	res= csa.wavelet_tree.range_search_2d((*it).first, (*it).second-1, 5, maxx).second;
	//might want to sort res based on pair.second - from some examples it looks like sdsl already does that so res is already sorted 
	uint32_t prev_num=0;
	for (auto z=res.begin();z!=res.end();++z) { 
	  uint64_t i=(*z).first;
	  uint32_t num=(*z).second;

	  if (num==prev_num) ignore=true;
	  else ignore=false;

	  left_new=(*it).first;
	  right_new=(*it).second;

	  //need original [l,r] to for the next loop iterations

	  left_rev_new=(*it_rev).first;
	  right_rev_new=(*it_rev).second;
       
	  if (num!=prev_num && num%2==1) {
	    if (num==(*(z+1)).second) {
	      left_new=csa.C[csa.char2comp[num]]; //need to modify left_rev_new as well?
	      right_new=left_new+2;
	    }
	    else {
	      left_new=i;
	      right_new=i+1;
	    }
	  }

	  last=skip(csa,left_new,right_new,left_rev_new,right_rev_new,num);
	
	  // how to alternate between forward and backward?
	  if (it==sa_intervals.begin() && first_del==false && !ignore) {

	    //this is the first site that the read overlaps
	    sa_intervals.push_back(std::make_pair(left_new,right_new));
	    sa_intervals_rev.push_back(std::make_pair(left_rev_new,right_rev_new));
	    get_location(csa,i,num,last, mask_a, tracker, prg_sites);
	  }
	    //there will be entries with pair.second empty (corresp to allele) coming from crossing the last marker
	    //can delete them here or in top a fcn when calculating coverages
	  else {
	    if (ignore) {
	      //second allele boundary crosses within one site
	      if (num%2==0)
		{
		  get_location(csa,i,num,last, mask_a, tracker, prg_sites);
		}
	      //else ?
	    }
	    else {

	      //read crossing a second site
	      *it=std::make_pair(left_new,right_new);
	      *it_rev=std::make_pair(left_rev_new,right_rev_new);
	      get_location(csa,i,num,last, mask_a, tracker, prg_sites);
	      //++it;
	      //++it_rev;
	      //++it_s;
	    }
	  }
	  prev_num=num;  
	}
	  j++;
	  ++it;
	  ++it_rev;
	  //++it_s;
      }
    }
    
    assert(sa_intervals.size()==sa_intervals_rev.size());
    if (tracker != NULL)
      {
	assert(sa_intervals.size()==tracker.vec.size());
      }
    it=sa_intervals.begin();
    it_rev=sa_intervals_rev.begin();	
    
    while (it!=sa_intervals.end() && it_rev!=sa_intervals_rev.end() ) {	
      //calculate sum to return- can do this in top fcns
      if (bidir_search(csa,(*it).first,(*it).second,(*it_rev).first,(*it_rev).second,c)>0) {
	++it;
	++it_rev;
      }
      else {
	if (it==sa_intervals.begin()) first_del=true;
	//might need to see first_del from top fcns to check if there are matches in the reference
	it=sa_intervals.erase(it);
	it_rev=sa_intervals_rev.erase(it_rev);
	//it_s=sites.erase(it_s);
	// QUESTION - TODO HERE FOR TRACKER??????????????????
      }
    }
  }
  
  if (pat_it!=pat_begin) return(pat_it); // where it got stuck
  else {
    if (!sa_intervals.empty()) return(pat_end);
    else return(pat_begin); //where it got stuck
  }
}			     			     			          
