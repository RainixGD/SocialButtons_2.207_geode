#include "./includes.h"
#include <Geode/modify/MenuLayer.hpp>

struct SocialButtonData {
	std::string link;
	std::string texture;
	bool isActive;
};

class SocialButton : public CCMenuItemSpriteExtra {
	std::string link;

	void onClick(CCObject*) {
		CCApplication::sharedApplication()->openURL(link.c_str());
	}

	virtual bool init(CCSprite* sprite, const std::string& link) {
		if (!CCMenuItemSpriteExtra::init(sprite, sprite, this, menu_selector(SocialButton::onClick))) return false;

		this->link = link;

		return true;
	}

public:

	static SocialButton* create(CCSprite* sprite, const std::string& link) {
		auto ret = new SocialButton();
		if (ret && ret->init(sprite, link)) {
			ret->autorelease();
			return ret;
		}
		delete ret;
		return nullptr;
	}
};

class SocialButtonsManager {

	enum DataLoadingResult {
		OK,
		FileNotFound,
		ParsingError,
		TooManyButtons
	};

	std::vector<SocialButtonData*> data;
	DataLoadingResult loadingStatus;
	static SocialButtonsManager* instance;
	
	void init() {
		loadingStatus = loadData();
	}

	DataLoadingResult loadData() {
		std::ifstream file("Resources/socialBtns.json");
		if (!file) return FileNotFound;
		std::ostringstream buffer;
		buffer << file.rdbuf();
		std::string fileContent = buffer.str();

		file.close();
		try {
			auto root = nlohmann::json::parse(fileContent);

			if (!root.contains("buttons") || !root["buttons"].is_array()) return ParsingError;

			auto buttons = root["buttons"];

			if (buttons.size() > 6) {
				return TooManyButtons;
			}

			for (const auto& btn : buttons) {
				if (!btn.contains("isBtn") || !btn["isBtn"].is_number_integer() ||
					!btn.contains("texture") || !btn["texture"].is_string() ||
					!btn.contains("link") || !btn["link"].is_string()) {
					return ParsingError;
				}

				bool isBtn = static_cast<bool>(btn["isBtn"].get<int>());
				std::string texture = btn["texture"];
				std::string link = btn["link"];

				auto buttonInfo = new SocialButtonData;
				buttonInfo->isActive = isBtn;
				buttonInfo->texture = texture;
				buttonInfo->link = link;
				data.push_back(buttonInfo);
			}
		}
		catch (...) {
			return ParsingError;
		}
		return OK;
	}

	SocialButtonsManager() {};
public:
	void onMenuLayer(MenuLayer* layer) {
		
		if (loadingStatus != OK) {

			std::string errorText;
			switch (loadingStatus){
			case SocialButtonsManager::FileNotFound:
				errorText = "Can't find 'socialBtns.json' in ./Resources";
				break;
			case SocialButtonsManager::ParsingError:
				errorText = "Can't parse 'socialBtns.json'";
				break;
			case SocialButtonsManager::TooManyButtons:
				errorText = "Too many buttons in 'socialBtns.json'";
				break;
			}

			auto size = CCDirector::sharedDirector()->getWinSize();

			auto errorLabel = CCLabelBMFont::create(errorText.c_str(), "bigFont.fnt");
			errorLabel->setColor({ 255, 0, 0 });
			errorLabel->setScale(0.6);
			errorLabel->setPosition({ size.width / 2, size.height - 10 });
			layer->addChild(errorLabel);

			return;
		}

		auto menu = layer->getChildByID("social-media-menu");
		menu->removeFromParent();

		CCMenu* upMenu = CCMenu::create();
		CCMenu* downMenu = CCMenu::create();
		layer->addChild(upMenu);
		layer->addChild(downMenu);
		upMenu->setPosition({ 60, 53 });
		downMenu->setPosition({ 60, 23 });

		for (int j = 0; j < data.size(); j++) {
			if (!data[j]->isActive)
				continue;

			auto btnSprite = CCSprite::create(data[j]->texture.c_str());
			if (btnSprite == NULL)
				btnSprite = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
			btnSprite->setScale(0.8);
			auto btn = SocialButton::create(btnSprite, data[j]->link);

			if (j < 4) upMenu->addChild(btn, 5, 100 + j);
			else downMenu->addChild(btn, 5, 100 + j);
		}
		upMenu->alignItemsHorizontallyWithPadding(5);
		downMenu->alignItemsHorizontallyWithPadding(5);
	}

	static SocialButtonsManager* getInstance() {
		if (!instance) {
			instance = new SocialButtonsManager();
			instance->init();
		}
		return instance;
	}

};
SocialButtonsManager* SocialButtonsManager::instance = nullptr;


class $modify(MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;

		SocialButtonsManager::getInstance()->onMenuLayer(this);
		return true;
	}
};