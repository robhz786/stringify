
StrfVersion = v0.14.0

all: \
    out/syntax.svg \
    out/tutorial.html \
    out/quick_reference.html \
    out/locale_hpp.html \
    out/benchmarks.html \
    out/howto_add_destination.html \
    out/howto_add_printable_types.html \
    out/howto_override_printable_types.html \
    out/cuda.html \
    out/versus_fmtlib.html \
    out/strf_hpp.html \
    out/outbuff_hpp.html \
    out/to_cfile_hpp.html \
    out/to_streambuf_hpp.html \
    out/to_string_hpp.html

formatters_abastracts := \
  alignment_formatter_abstract.adoc \
  int_formatter_abstract.adoc \
  float_formatter_abstract.adoc

out/ :
	mkdir -p out

out/syntax.svg : syntax.svg out/
	cp $< $@

out/tutorial.html : tutorial.adoc out/
	asciidoctor -v $< -o $@

out/quick_reference.html : quick_reference.adoc $(formatters_abastracts) out/
	asciidoctor -v $< -o $@

out/locale_hpp.html : locale_hpp.adoc out/
	asciidoctor -v $< -o $@

out/benchmarks.html : benchmarks.adoc out/
	asciidoctor -v --attribute=strf-version=$(StrfVersion) $< -o $@

out/howto_add_destination.html : howto_add_destination.adoc out/
	asciidoctor -v --attribute=strf-version=$(StrfVersion) $< -o $@

out/howto_add_printable_types.html : howto_add_printable_types.adoc out/
	asciidoctor -v --attribute=strf-version=$(StrfVersion) $< -o $@

out/howto_override_printable_types.html : howto_override_printable_types.adoc out/
	asciidoctor -v --attribute=strf-version=$(StrfVersion) $< -o $@

out/cuda.html : cuda.adoc out/
	asciidoctor -v --attribute=strf-version=$(StrfVersion) $< -o $@

out/versus_fmtlib.html : versus_fmtlib.adoc out/
	asciidoctor -v --attribute=strf-version=$(StrfVersion) $< -o $@

out/strf_hpp.html : strf_hpp.adoc \
                    ref_facets_pack.adoc \
                    ref_printer.adoc \
                    ref_format_functions.adoc \
                    ref_printable_types.adoc \
                    ref_destination.adoc \
                    ref_tr-string.adoc \
                    ref_char_encoding.adoc \
                    ref_width.adoc \
                    ref_numpunct.adoc \
                    ref_misc.adoc \
                    $(formatters_abastracts) \
                    out /
	asciidoctor -v $< -o - | sed 's/20em/34em/g' | sed 's/td.hdlist1{/td.hdlist1{min-width:9em;/g' > $@

out/outbuff_hpp.html : outbuff_hpp.adoc out/
	asciidoctor -v $< -o - | sed 's/20em/34em/g' | sed 's/td.hdlist1{/td.hdlist1{min-width:9em;/g' > $@

out/to_cfile_hpp.html : to_cfile_hpp.adoc out/
	asciidoctor -v $< -o - | sed 's/20em/34em/g' | sed 's/td.hdlist1{/td.hdlist1{min-width:9em;/g' > $@

out/to_streambuf_hpp.html : to_streambuf_hpp.adoc out/
	asciidoctor -v $< -o - | sed 's/20em/34em/g' | sed 's/td.hdlist1{/td.hdlist1{min-width:9em;/g' > $@

out/to_string_hpp.html : to_string_hpp.adoc out/
	asciidoctor -v $< -o - | sed 's/20em/34em/g' | sed 's/td.hdlist1{/td.hdlist1{min-width:9em;/g' > $@

