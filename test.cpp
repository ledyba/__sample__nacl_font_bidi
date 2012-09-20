#include <iostream>
#include <unicode/ubidi.h>
#include <unicode/utypes.h>
#include <unicode/putil.h>
#include <unicode/uiter.h>
#include <unicode/stringpiece.h>
#include <unicode/utf8.h>
#include <unicode/uchar.h>
#include <iconv.h>
#include <string.h>
#include <unicode/ucnv.h>

// Splits a PP_BrowserFont_Trusted_TextRun into a sequence or LTR and RTL
// WebTextRuns that can be used for WebKit. Normally WebKit does this for us,
// but the font drawing and measurement routines we call happen after this
// step. So for correct rendering of RTL content, we need to do it ourselves.
class TextRunCollection {
	public:
		explicit TextRunCollection(const char* text)
			: bidi_(NULL),
			text_(text, strlen(text), "UTF-8"),
			num_runs_(0) {

				bidi_ = ubidi_open();
				UErrorCode uerror = U_ZERO_ERROR;
				ubidi_setPara(bidi_, text_.getBuffer(), text_.length(), UBIDI_DEFAULT_RTL, NULL, &uerror);
				if (U_SUCCESS(uerror)){
					num_runs_ = ubidi_countRuns(bidi_, &uerror);
				}
			}

		~TextRunCollection() {
			if (bidi_)
				ubidi_close(bidi_);
		}

		int num_runs() const { return num_runs_; }

		// Returns a WebTextRun with the info for the run at the given index.
		// The range covered by the run is in the two output params.
		UChar* GetRunAt(int index, int32_t* run_start, int32_t* run_len) const {
			if (bidi_) {
				bool run_rtl = !!ubidi_getVisualRun(bidi_, index, run_start, run_len);
				return (UChar*)&text_.getBuffer()[*run_start];
			}

			return 0;
		}

	private:
		// Will be null if we skipped autodetection.
		UBiDi* bidi_;

		// Text of all the runs.
		const icu::UnicodeString text_;
		

		int num_runs_;
};

#include <unicode/bytestream.h>
int main(int argc, char** argv)
{
	TextRunCollection runs("█ おおアイ明朝\u2588\u3000全角スペース");
	std::cout << "num_runs: " << runs.num_runs() << std::endl;
	char buf[8192];
	CheckedArrayByteSink sink (buf, 8192);
	for (int i = 0; i < runs.num_runs(); i++) {
		int32_t run_begin = 0;
		int32_t run_len = 0;
		UChar* c = runs.GetRunAt(i, &run_begin, &run_len);
		icu::UnicodeString(c, run_len).toUTF8(sink);
		std::cout << buf << std::endl;
	}
	return 0;
}

