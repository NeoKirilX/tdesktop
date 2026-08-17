/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "settings/sections/settings_exteragram.h"

#include "settings/settings_common_session.h"

#include "core/click_handler_types.h"
#include "lang/lang_keys.h"
#include "main/main_session.h"
#include "settings/settings_builder.h"
#include "settings/sections/settings_main.h"
#include "ui/basic_click_handlers.h"
#include "ui/painter.h"
#include "ui/rect.h"
#include "ui/vertical_list.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_controller.h"
#include "window/window_session_controller.h"
#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"

namespace Settings {
namespace {

using namespace Builder;

constexpr auto kExteraGramChannel = "https://t.me/Exteragram_tdesktop"_q;
constexpr auto kExteraGramChat = "https://t.me/+oZKYqEKf9BljYzQy"_q;

constexpr auto kPhotoSize = 100;
constexpr auto kCardHeight = 80;
constexpr auto kButtonWidth = 80;
constexpr auto kButtonHeight = 30;

struct PluginInfo {
	QString id;
	QString name;
	QString author;
	QString version;
	QString description;
	QString stickerEmoji;
	bool installed = false;
	bool enabled = false;
};

std::vector<PluginInfo> &AllPlugins() {
	static auto result = std::vector<PluginInfo>{
		{
			.id = u"example_plugin_1"_q,
			.name = u"QR Code Generator"_q,
			.author = u"exteraDev"_q,
			.version = u"1.0.0"_q,
			.description = u"Generates QR codes from any text or link right in the chat." _q,
			.stickerEmoji = u"\xF0\x9F\x92\xBB"_q,
		},
		{
			.id = u"example_plugin_2"_q,
			.name = u"Custom FAB"_q,
			.author = u"itsv1eds"_q,
			.version = u"1.2.0"_q,
			.description = u"Change the floating action button in the chat list with custom actions." _q,
			.stickerEmoji = u"\xF0\x9F\x92\xA1"_q,
		},
		{
			.id = u"example_plugin_3"_q,
			.name = u"Now Playing"_q,
			.author = u"Nightly"_q,
			.version = u"2.0.0"_q,
			.description = u"Show your currently playing track from Spotify or Last.fm in your profile status." _q,
			.stickerEmoji = u"\xF0\x9F\x8E\xB5"_q,
		},
	};
	return result;
}

class ExteraGramCover final : public Ui::FixedHeightWidget {
public:
	ExteraGramCover(QWidget *parent);

private:
	void paintEvent(QPaintEvent *e) override;

};

ExteraGramCover::ExteraGramCover(QWidget *parent)
: FixedHeightWidget(
	parent,
	st::settingsPhotoTop
		+ kPhotoSize
		+ st::settingsPhotoBottom) {
}

void ExteraGramCover::paintEvent(QPaintEvent *) {
	auto p = QPainter(this);

	const auto x = st::settingsPhotoLeft;
	const auto y = st::settingsPhotoTop;

	PainterHighQualityEnabler hq(p);
	p.setPen(Qt::NoPen);
	p.setBrush(st::windowBgActive);
	p.drawEllipse(
		x,
		y,
		kPhotoSize,
		kPhotoSize);

	p.setPen(st::windowFgActive);
	p.setFont(st::normalFont);
	p.drawText(
	 QRect(x, y, kPhotoSize, kPhotoSize),
		Qt::AlignCenter,
		u"E"_q);
}

class PluginCard final : public Ui::RpWidget {
public:
	PluginCard(
		QWidget *parent,
		PluginInfo *info,
		Fn<void()> reloadCallback);

private:
	void paintEvent(QPaintEvent *e) override;
	void mousePressEvent(QMouseEvent *e) override;

	const not_null<PluginInfo*> _info;
	Fn<void()> _reload;

};

PluginCard::PluginCard(
	QWidget *parent,
	PluginInfo *info,
	Fn<void()> reloadCallback)
: RpWidget(parent)
, _info(info)
, _reload(std::move(reloadCallback)) {
	resize(0, kCardHeight);
}

void PluginCard::paintEvent(QPaintEvent *) {
	auto p = QPainter(this);
	const auto w = width();
	const auto h = height();

	const auto cursorPos = mapFromGlobal(QCursor::pos());
	if (rect().contains(cursorPos)) {
		p.fillRect(rect(), st::windowBgOver);
	}

	const auto photoSize = 48;
	const auto photoX = st::boxRowPadding.left();
	const auto photoY = (h - photoSize) / 2;

	PainterHighQualityEnabler hq(p);
	p.setPen(Qt::NoPen);
	p.setBrush(st::windowBgActive);
	p.drawEllipse(
		QPoint(photoX + photoSize / 2, photoY + photoSize / 2),
		photoSize / 2,
		photoSize / 2);

	p.setPen(st::windowFgActive);
	p.setFont(st::normalFont);
	p.drawText(
		QRect(QPoint(photoX, photoY), QSize(photoSize, photoSize)),
		Qt::AlignCenter,
		_info->stickerEmoji);

	const auto nameX = photoX + photoSize + st::boxRowPadding.left();
	const auto nameY = photoY + 4;
	p.setPen(st::windowFg);
	p.setFont(st::normalFont);
	p.drawText(
		nameX,
		nameY + st::normalFont->ascent,
		_info->name);

	const auto versionAuthor = _info->version
		+ u" \xB7 "_q
		+ _info->author;
	p.setPen(st::windowSubTextFg);
	p.setFont(st::normalFont);
	p.drawText(
		nameX,
		nameY + st::normalFont->height + st::normalFont->ascent + 2,
		versionAuthor);

	const auto buttonX = w
		- st::boxRowPadding.right()
		- kButtonWidth;
	const auto buttonY = (h - kButtonHeight) / 2;

	const auto buttonColor = _info->installed
		? (_info->enabled
			? st::windowBgActive
			: st::windowSubTextFg)
		: st::windowBgActive;
	p.setPen(Qt::NoPen);
	p.setBrush(buttonColor);
	p.drawRoundedRect(
		buttonX,
		buttonY,
		kButtonWidth,
		kButtonHeight,
		st::buttonRadius,
		st::buttonRadius);

	p.setPen(_info->installed ? st::windowBg : st::windowFg);
	p.setFont(st::normalFont);
	const auto buttonText = _info->installed
		? (_info->enabled ? u"Enabled"_q : u"Enable"_q)
		: u"Install"_q;
	p.drawText(
		QRect(QPoint(buttonX, buttonY), QSize(kButtonWidth, kButtonHeight)),
		Qt::AlignCenter,
		buttonText);
}

void PluginCard::mousePressEvent(QMouseEvent *e) {
	if (e->button() != Qt::LeftButton) {
		return;
	}

	const auto w = width();
	const auto h = height();
	const auto buttonX = w
		- st::boxRowPadding.right()
		- kButtonWidth;
	const auto buttonY = (h - kButtonHeight) / 2;

	const auto clickPos = e->pos();
	if (clickPos.x() >= buttonX
		&& clickPos.x() <= buttonX + kButtonWidth
		&& clickPos.y() >= buttonY
		&& clickPos.y() <= buttonY + kButtonHeight) {
		if (!_info->installed) {
			_info->installed = true;
			_info->enabled = true;
		} else {
			_info->enabled = !_info->enabled;
		}
		update();
		if (_reload) {
			_reload();
		}
		return;
	}
}

void BuildPluginsSection(SectionBuilder &builder) {
	builder.addSubsectionTitle(rpl::single(u"Plugins"_q));

	const auto plugins = &AllPlugins();
	for (auto i = 0, count = int(plugins->size()); i < count; ++i) {
		const auto info = &(*plugins)[i];
		builder.add([=](const WidgetContext &ctx) {
			auto card = object_ptr<PluginCard>(
				ctx.container,
				info,
				[=] {
					if (const auto w = ctx.controller->window().widget()) {
						w->update();
					}
				});
			return SectionBuilder::WidgetToAdd{
				.widget = std::move(card),
				.margin = QMargins(
					-st::boxRowPadding.left(),
					0,
					-st::boxRowPadding.right(),
					0),
			};
		});
	}

	builder.addSkip();
}

void BuildLinksSection(SectionBuilder &builder) {
	builder.addSubsectionTitle(rpl::single(u"Links"_q));

	builder.addButton({
		.id = u"exteragram/channel"_q,
		.title = rpl::single(u"ExteraGram Desktop Channel"_q),
		.icon = { &st::menuIconChannel },
		.onClick = [=] {
			UrlClickHandler::Open(kExteraGramChannel);
		},
		.keywords = { u"channel"_q, u"updates"_q },
	});

	builder.addButton({
		.id = u"exteragram/chat"_q,
		.title = rpl::single(u"ExteraGram Desktop Chat"_q),
		.icon = { &st::menuIconChatDiscuss },
		.onClick = [=] {
			UrlClickHandler::Open(kExteraGramChat);
		},
		.keywords = { u"chat"_q, u"community"_q, u"group"_q },
	});

	builder.addSkip();
}

class ExteraGram final : public Section<ExteraGram> {
public:
	ExteraGram(
		QWidget *parent,
		not_null<Window::SessionController*> controller);

	[[nodiscard]] rpl::producer<QString> title() override;

private:
	void setupContent();

};

ExteraGram::ExteraGram(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section(parent, controller) {
	setupContent();
}

rpl::producer<QString> ExteraGram::title() {
	return rpl::single(u"ExteraGram Preferences"_q);
}

const auto kMeta = BuildHelper({
	.id = ExteraGram::Id(),
	.parentId = MainId(),
	.title = &tr::lng_menu_settings,
	.icon = &st::menuIconSettings,
}, [](SectionBuilder &builder) {
	builder.addDivider();
	builder.addSkip();

	BuildPluginsSection(builder);
	BuildLinksSection(builder);
});

const SectionBuildMethod kExteraGramSection = kMeta.build;

void ExteraGram::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);

	content->add(object_ptr<ExteraGramCover>(content));

	const auto nameLeft = st::settingsPhotoLeft
		+ kPhotoSize
		+ st::boxRowPadding.left();

	content->add(
		object_ptr<Ui::FlatLabel>(
			content,
			rpl::single(u"ExteraGram"_q),
			st::defaultFlatLabel),
		QMargins(
			nameLeft,
			st::settingsNameTop,
			0,
			0));

	content->add(
		object_ptr<Ui::FlatLabel>(
			content,
			rpl::single(u"Telegram Desktop with extra features"_q),
			st::defaultFlatLabel),
		QMargins(
			nameLeft,
			0,
			0,
			0));

	Ui::AddSkip(content);
	Ui::AddDivider(content);
	Ui::AddSkip(content);

	build(content, kExteraGramSection);

	Ui::ResizeFitChild(this, content);
}

} // namespace

Type ExteraGramId() {
	return ExteraGram::Id();
}

} // namespace Settings
