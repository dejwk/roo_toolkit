#include "roo_toolkit/log/log.h"

#include "roo_display/shape/basic_shapes.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"
#include "roo_smooth_fonts/NotoSans_Condensed/18.h"
#include "roo_windows/core/panel.h"

namespace roo_toolkit {
namespace log {

using namespace roo_display;
using namespace roo_windows;

Log::Log(const roo_windows::Environment& env, uint32_t buffer_size,
         size_t max_lines)
    : roo_windows::BasicWidget(env),
      buffer_size_(buffer_size),
      buffer_(new uint8_t[buffer_size]),
      font_(&font_NotoSans_Condensed_18()),
      cursor_(buffer_.get()),
      lines_(max_lines),
      line_widths_(max_lines),
      max_line_width_(0),
      line_start_(0),
      line_count_(0) {
  setPadding(PaddingSize::PADDING_TINY);
}

void Log::clear() {
  cursor_ = buffer_.get();
  line_start_ = 0;
  line_count_ = 0;
  invalidateInterior();
}

int Log::line_pos(int idx) const {
  idx += line_start_;
  if (idx > lines_.size()) idx -= lines_.size();
  return idx;
}

void Log::drop_first_line() {
  if (line_widths_[line_start_] == max_line_width_) {
    max_line_width_ = -1;
  }
  --line_count_;
  ++line_start_;
  if (line_start_ == lines_.size()) line_start_ = 0;
  markDirty();
}

void Log::setFont(const roo_display::Font* font) {
  if (font_ == font) return;
  font_ = font;
  if (line_count_ > 0) {
    // Recalculate line widths.
    max_line_width_ = 0;
    for (int i = 0; i < line_count_; ++i) {
      int target_idx = line_pos(i);
      GlyphMetrics m = font_->getHorizontalStringMetrics(lines_[target_idx]);
      line_widths_[target_idx] = m.advance();
      if (max_line_width_ < m.advance()) max_line_width_ = m.advance();
    }
    invalidateInterior();
  }
  requestLayout();
}

Dimensions Log::getSuggestedMinimumDimensions() const {
  return Dimensions(maxLineWidth(), line_count_ * font_->metrics().linespace());
}

PreferredSize Log::getPreferredSize() const {
  return PreferredSize(
      PreferredSize::MatchParent(),
      PreferredSize::Exact(line_count_ * font_->metrics().linespace()));
}

Dimensions Log::onMeasure(MeasureSpec width, MeasureSpec height) {
  Padding p = getPadding();
  return Dimensions(
      width.resolveSize(maxLineWidth() + p.left() + p.right()),
      height.resolveSize(line_count_ * font_->metrics().linespace() + p.top() +
                         p.bottom()));
}

void Log::appendLine(roo_display::StringView line) {
  int old_line_count = line_count_;
  while (line_count_ > 0 && lines_[line_start_].data() > cursor_ &&
         lines_[line_start_].data() - cursor_ < line.size()) {
    drop_first_line();
  }
  if ((cursor_ - buffer_.get()) + line.size() > buffer_size_) {
    // Won't fit; we need to make space from the top of the buffer.
    cursor_ = buffer_.get();
    while (line_count_ > 0 &&
           lines_[line_start_].data() < cursor_ + line.size()) {
      drop_first_line();
    }
  }
  while (line_count_ > lines_.size()) {
    drop_first_line();
  }
  size_t s = line.size();
  size_t space = buffer_.get() + buffer_size_ - cursor_;
  if (s > space) s = space;
  memcpy(cursor_, line.data(), s);
  int target_idx = line_pos(line_count_);
  lines_[target_idx] = roo_display::StringView(cursor_, s);
  markDirty(Box(0, line_count_ * font_->metrics().linespace(), width() - 1,
                (line_count_ + 1) * font_->metrics().linespace() - 1));
  ++line_count_;
  cursor_ += s;
  if (old_line_count != line_count_) {
    requestLayout();
  }
}

int16_t Log::maxLineWidth() const {
  if (max_line_width_ < 0) {
    max_line_width_ = 0;
    for (int i = 0; i < line_count_; i++) {
      int pos = line_pos(i);
      if (max_line_width_ < line_widths_[pos]) {
        max_line_width_ = line_widths_[pos];
      }
    }
  }
  return max_line_width_;
}

bool Log::paint(const Surface& s) {
  Padding p = getPadding();
  int first_visible_line =
      (s.clip_box().yMin() - s.dy() - p.top()) / font_->metrics().linespace();
  if (first_visible_line <= 0) {
    s.drawObject(
        FilledRect(0, 0, width() - 1, p.top() - 1, color::Transparent));
    first_visible_line = 0;
  }
  int last_visible_line = (s.clip_box().yMax() - s.dy() - p.top() - 1) /
                              font_->metrics().linespace() +
                          1;
  if (last_visible_line < 0) return true;
  if (last_visible_line >= line_count_) last_visible_line = line_count_ - 1;
  if (first_visible_line > last_visible_line) {
    s.drawObject(Clear());
    return true;
  }
  int16_t y = first_visible_line * font_->metrics().linespace() + p.top();
  for (int i = first_visible_line; i <= last_visible_line; ++i) {
    s.drawObject(MakeTileOf(
        StringViewLabel(lines_[line_pos(i)], *font_, parent()->defaultColor()),
        Box(0, y, width() - 1, y + font_->metrics().linespace() - 1),
        kLeft.shiftBy(p.left()) | kMiddle));
    y += font_->metrics().linespace();
  }
  if (y < height() - 1) {
    s.drawObject(
        FilledRect(0, y, width() - 1, height() - 1, color::Transparent));
  }
  return true;
}

}  // namespace log
}  // namespace roo_toolkit
