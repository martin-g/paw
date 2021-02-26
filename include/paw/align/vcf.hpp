#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <paw/align/variant.hpp>


namespace paw
{


class Vcf
{
private:
  std::string filename;
  std::vector<Variant> vars;
  std::vector<std::string> sample_names;

public:
  std::string reference{};
  std::string chrom{};

  Vcf(std::string const & fn);
  void add_variant(Variant const & var);
  void add_sample_name(std::string const & sample_name);
  void write_header(std::stringstream & ss);
  void write_record(std::stringstream & ss, Variant const & var);
  void write();

};


} // namespace paw


#if defined(IMPLEMENT_PAW) || defined(__JETBRAINS_IDE__)


namespace paw
{

Vcf::Vcf(std::string const & fn)
  : filename(fn)
{}


void
Vcf::add_sample_name(std::string const & sample_name)
{
  sample_names.push_back(sample_name);
}


void
Vcf::add_variant(Variant const & var)
{
  vars.push_back(var);
}


void
Vcf::write_header(std::stringstream & ss)
{
  ss << "##fileformat=VCFv4.2\n";

  if (chrom.size() > 0)
    ss << "##contig=<ID=" << chrom << ">\n";
  else if (sample_names.size() > 0)
    ss << "##contig=<ID=N" << sample_names[0] << ">\n";
  else
    ss << "##contig=<ID=chr1>\n";

  ss << "##INFO=<ID=FEATURE,Number=1,Type=String,Description=\"Gene feature.\">\n";
  ss << "##INFO=<ID=FEATURE_NUM,Number=1,Type=String,Description=\"Gene feature number.\">\n";
  ss << "##FORMAT=<ID=GT,Number=1,Type=String,Description=\"Genotype\">\n";
  ss << "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT";

  for (auto const & sn : sample_names)
    ss << "\t" << sn;

  ss << "\n";
}


void
Vcf::write_record(std::stringstream & ss, Variant const & var)
{
  assert(sample_names.size() == var.calls.size());

  if (chrom.size() > 0)
    ss << chrom;
  else if (sample_names.size() > 0)
    ss << "N" << sample_names[0];
  else
    ss << "chr1";

  // POS
  ss << "\t" << (var.pos + 1) << "\t.\t" << var.seqs[0] << "\t" << var.seqs[1];

  for (std::size_t i = 2; i < var.seqs.size(); ++i)
  {
    if (var.seqs[i].size() == 0)
    {
      std::cerr << "[paw::align::vcf] ERROR: empty allele\n";
      std::exit(1);
    }

    ss << "," << var.seqs[i];
  }

  ss << "\t0\t.\t";

  // Parse info
  if (var.infos.empty())
  {
    ss << ".";
  }
  else
  {
    auto write_info = [&](std::map<std::string, std::string>::const_iterator it)
                      {
                        ss << it->first;

                        if (it->second.size() > 0)
                          ss << '=' << it->second;
                      };

    write_info(var.infos.cbegin());

    for (auto map_it = std::next(var.infos.cbegin(), 1); map_it != var.infos.cend(); ++map_it)
    {
      ss << ';';
      write_info(map_it);
    }
  }

  ss << "\tGT";

  for (auto const call : var.calls)
    ss << "\t" << call;

  ss << "\n";
}


void
Vcf::write()
{
  std::stringstream ss;

  write_header(ss);

  for (auto const & var : vars)
    write_record(ss, var);

  if (filename == "-")
  {
    std::cout << ss.rdbuf();
  }
  else
  {
    std::ofstream ofile(filename);
    ofile << ss.rdbuf();
  }
}


} // namespace paw


#endif // IMPLEMENT_PAW
