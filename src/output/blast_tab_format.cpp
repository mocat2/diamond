/****
Copyright (c) 2016, Benjamin Buchfink
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****/

#include "../basic/match.h"
#include "output_format.h"

const char* Blast_tab_format::field_str[] = {
	"qseqid",		// 0 means Query Seq - id
	"qgi",			// 1 means Query GI
	"qacc",			// 2 means Query accesion
	"qaccver",		// 3 means Query accesion.version
	"qlen",			// 4 means Query sequence length
	"sseqid",		// 5 means Subject Seq - id
	"sallseqid",	// 6 means All subject Seq - id(s), separated by a ';'
	"sgi",			// 7 means Subject GI
	"sallgi",		// 8 means All subject GIs
	"sacc",			// 9 means Subject accession
	"saccver",		// 10 means Subject accession.version
	"sallacc",		// 11 means All subject accessions
	"slen",			// 12 means Subject sequence length
	"qstart",		// 13 means Start of alignment in query
	"qend",			// 14 means End of alignment in query
	"sstart",		// 15 means Start of alignment in subject
	"send",			// 16 means End of alignment in subject
	"qseq",			// 17 means Aligned part of query sequence
	"sseq",			// 18 means Aligned part of subject sequence
	"evalue",		// 19 means Expect value
	"bitscore",		// 20 means Bit score
	"score",		// 21 means Raw score
	"length",		// 22 means Alignment length
	"pident",		// 23 means Percentage of identical matches
	"nident",		// 24 means Number of identical matches
	"mismatch",		// 25 means Number of mismatches
	"positive",		// 26 means Number of positive - scoring matches
	"gapopen",		// 27 means Number of gap openings
	"gaps",			// 28 means Total number of gaps
	"ppos",			// 29 means Percentage of positive - scoring matches
	"frames",		// 30 means Query and subject frames separated by a '/'
	"qframe",		// 31 means Query frame
	"sframe",		// 32 means Subject frame
	"btop",			// 33 means Blast traceback operations(BTOP)
	"staxids",		// 34 means unique Subject Taxonomy ID(s), separated by a ';'	(in numerical order)
	"sscinames",	// 35 means unique Subject Scientific Name(s), separated by a ';'
	"scomnames",	// 36 means unique Subject Common Name(s), separated by a ';'
	"sblastnames",	// 37 means unique Subject Blast Name(s), separated by a ';'	(in alphabetical order)
	"sskingdoms",	// 38 means unique Subject Super Kingdom(s), separated by a ';'	(in alphabetical order)
	"stitle",		// 39 means Subject Title
	"salltitles",	// 40 means All Subject Title(s), separated by a '<>'
	"sstrand",		// 41 means Subject Strand
	"qcovs",		// 42 means Query Coverage Per Subject
	"qcovhsp",		// 43 means Query Coverage Per HSP
	"qcovus",		// 44 means Query Coverage Per Unique Subject(blastn only)
	"qtitle"		// 45 means Query title
};

Blast_tab_format::Blast_tab_format() :
	Output_format(blast_tab)
{
	static const unsigned stdf[] = { 0, 5, 23, 22, 25, 27, 13, 14, 15, 16, 19, 20 };
	const vector<string> &f = config.output_format;
	if (f.size() <= 1) {
		fields = vector<unsigned>(stdf, stdf + 12);
		return;
	}
	for (vector<string>::const_iterator i = f.begin() + 1; i != f.end(); ++i) {
		int j = get_idx(field_str, sizeof(field_str) / sizeof(field_str[0]), i->c_str());
		if(j == -1)
			throw std::runtime_error(string("Invalid output field: ") + *i);
		fields.push_back(j);
		if (j == 6 || j == 39 || j == 40)
			config.salltitles = true;
	}
}

void Blast_tab_format::print_match(const Hsp_context& r, Text_buffer &out) const
{
	for (vector<unsigned>::const_iterator i = fields.begin(); i != fields.end(); ++i) {
		switch (*i) {
		case 0:
			out.write_until(r.query_name, Const::id_delimiters);
			break;
		case 4:
			out << r.source_query.length();
			break;
		case 5:
			this->print_salltitles(out, r.subject_name, false, false);
			break;
		case 6:
			this->print_salltitles(out, r.subject_name, false, true);
			break;
		case 12:
			out << r.subject_len;
			break;
		case 13:
			out << r.oriented_query_range().begin_ + 1;
			break;
		case 14:
			out << r.oriented_query_range().end_ + 1;
			break;
		case 15:
			out << r.subject_range().begin_ + 1;
			break;
		case 16:
			out << r.subject_range().end_;
			break;
		case 17:
			r.source_query.print(out, r.query_source_range().begin_, r.query_source_range().end_, input_value_traits);
			break;
		case 18:
		{
			vector<Letter> seq;
			seq.reserve(r.subject_range().length());
			for (Hsp_context::Iterator j = r.begin(); j.good(); ++j)
				if (!(j.op() == op_insertion))
					seq.push_back(j.subject());
			out << sequence(seq);
			break;
		}
		case 19:
			out.print_e(r.evalue());
			break;
		case 20:
			out << r.bit_score();
			break;
		case 21:
			out << r.score();
			break;
		case 22:
			out << r.length();
			break;
		case 23:
			out << (double)r.identities() * 100 / r.length();
			break;
		case 24:
			out << r.identities();
			break;
		case 25:
			out << r.mismatches();
			break;
		case 26:
			out << r.positives();
			break;
		case 27:
			out << r.gap_openings();
			break;
		case 28:
			out << r.gaps();
			break;
		case 29:
			out << (double)r.positives() * 100.0 / r.length();
			break;
		case 31:
			out << r.blast_query_frame();
			break;
		case 33:
		{
			unsigned n_matches = 0;
			for (Hsp_context::Iterator i = r.begin(); i.good(); ++i) {
				switch (i.op()) {
				case op_match:
					++n_matches;
					break;
				case op_substitution:
					if (n_matches > 0) {
						out << n_matches;
						n_matches = 0;
					}
					out << i.query_char() << i.subject_char();
					break;
				case op_insertion:
					if (n_matches > 0) {
						out << n_matches;
						n_matches = 0;
					}
					out << i.query_char() << '-';
					break;
				case op_deletion:
					if (n_matches > 0) {
						out << n_matches;
						n_matches = 0;
					}
					out << '-' << i.subject_char();
					break;
				}
			}
			if (n_matches > 0)
				out << n_matches;
		}
			break;
		case 39:
			this->print_salltitles(out, r.subject_name, true, false);
			break;
		case 40:
			this->print_salltitles(out, r.subject_name, true, true);
			break;
		case 43:
			out << (double)r.query_source_range().length()*100.0 / r.source_query.length();
			break;
		case 45:
			out << r.query_name;
			break;
		default:
			throw std::runtime_error("Invalid output field");
		}
		if (i < fields.end() - 1)
			out << '\t';
	}
	out << '\n';
}