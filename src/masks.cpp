#include <stdint.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "masks.hpp"


MasksParser::MasksParser(const std::string &sites_fname, const std::string &alleles_fname) {
    std::ifstream sites_stream(sites_fname);
    parse_sites(sites_stream);

    std::ifstream alleles_stream(alleles_fname);
    parse_allele(alleles_stream);
}


void MasksParser::parse_sites(std::istream &stream) {
    uint64_t max_sites_count = 0;
    uint64_t site_count;

    while (stream >> site_count) {
        MasksParser::sites.push_back(site_count);
        if (site_count > max_sites_count)
            max_sites_count = site_count;
    }

    // no_sites is last odd number in mask_sites, but alphabet size
    // is the even number corresponding to it
    MasksParser::max_alphabet_num = max_sites_count + 1;
}


void MasksParser::parse_allele(std::istream &stream) {
    int allele_count = 0;
    int a;

    // TODO: Assert at least 2 alleles at each site
    while (stream >> a) {
        if (a > allele_count)
            allele_count = a;
        if (a < allele_count && a != 0) {
            MasksParser::allele_coverage.push_back(std::vector<int>(allele_count, 0));
            allele_count = a;
        }
        MasksParser::allele.push_back(a);
    }

    if (allele_count > 0)
        MasksParser::allele_coverage.push_back(std::vector<int>(allele_count, 0));
}
