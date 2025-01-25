#include "./includes.h"
#include <Geode/modify/MenuLayer.hpp>
#include "./ErrorsManager/ErrorsManager.h"


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

	bool isOk = false;
	std::vector<SocialButtonData*> data;
	static SocialButtonsManager* instance;
	
	void init() {
		isOk = loadData();
	}

	bool isValidURL(const std::string& url) {
		std::regex url_regex(R"(^https?://)");
		return std::regex_search(url, url_regex);
	}

	bool hasFileExtension(const std::string& fileName, const std::string& extension) {
		std::string pattern = ".*\\." + extension + "$";
		std::regex regexPattern(pattern, std::regex::icase);
		return std::regex_match(fileName, regexPattern);
	}

    bool loadData() {
        std::ifstream file("Resources/socialBtns.json");
        if (!file) {
            ErrorsManager::addError("Social Buttons: Unable to open file 'Resources/socialBtns.json'. File not found.", ErrorsManager::Error);
            return false;
        }

        std::ostringstream buffer;
        try {
            buffer << file.rdbuf();
        }
        catch (const std::ios_base::failure& e) {
            ErrorsManager::addError("Social Buttons: Failed to read from file 'Resources/socialBtns.json': " + std::string(e.what()), ErrorsManager::Error);
            file.close();
            return false;
        }

        std::string fileContent = buffer.str();
        file.close();

        if (fileContent.empty()) {
            ErrorsManager::addError("Social Buttons: File 'Resources/socialBtns.json' is empty.", ErrorsManager::Error);
            return false;
        }

        try {
            auto root = nlohmann::json::parse(fileContent);

            if (!root.contains("buttons")) {
                ErrorsManager::addError("Social Buttons: JSON does not contain 'buttons' key.", ErrorsManager::Error);
                return false;
            }

            if (!root["buttons"].is_array()) {
                ErrorsManager::addError("Social Buttons: 'buttons' in JSON is not an array.", ErrorsManager::Error);
                return false;
            }

            auto buttons = root["buttons"];
            if (buttons.size() > 6) {
                ErrorsManager::addError("Social Buttons: Too many buttons in JSON. Maximum allowed is 6.", ErrorsManager::Error);
                return false;
            }

            for (size_t i = 0; i < buttons.size(); ++i) {
                const auto& btn = buttons[i];

                if (!btn.contains("isBtn")) {
                    ErrorsManager::addError("Social Buttons: Button at index " + std::to_string(i) + " is missing 'isBtn' property.", ErrorsManager::Error);
                    return false;
                }

                if (!btn["isBtn"].is_number_integer()) {
                    ErrorsManager::addError("Social Buttons: 'isBtn' property at index " + std::to_string(i) + " must be an integer.", ErrorsManager::Error);
                    return false;
                }

                if (!btn.contains("texture")) {
                    ErrorsManager::addError("Social Buttons: Button at index " + std::to_string(i) + " is missing 'texture' property.", ErrorsManager::Error);
                    return false;
                }

                if (!btn["texture"].is_string()) {
                    ErrorsManager::addError("Social Buttons: 'texture' property at index " + std::to_string(i) + " must be a string.", ErrorsManager::Error);
                    return false;
                }

                if (!btn.contains("link")) {
                    ErrorsManager::addError("Social Buttons: Button at index " + std::to_string(i) + " is missing 'link' property.", ErrorsManager::Error);
                    return false;
                }

                if (!btn["link"].is_string()) {
                    ErrorsManager::addError("Social Buttons: 'link' property at index " + std::to_string(i) + " must be a string.", ErrorsManager::Error);
                    return false;
                }

                bool isBtn = static_cast<bool>(btn["isBtn"].get<int>());
                std::string texture = btn["texture"];
                std::string link = btn["link"];

                if (!hasFileExtension(texture, "png")) {
                    ErrorsManager::addError("Social Buttons: Texture file for button at index " + std::to_string(i) + " must have a '.png' extension.", ErrorsManager::Error);
                    return false;
                }

                if (!isValidURL(link)) {
                    ErrorsManager::addError("Social Buttons: Link for button at index " + std::to_string(i) + " must be a valid URL starting with 'http://' or 'https://'.", ErrorsManager::Error);
                    return false;
                }

                try {
                    auto buttonInfo = new SocialButtonData;
                    buttonInfo->isActive = isBtn;
                    buttonInfo->texture = texture;
                    buttonInfo->link = link;
                    data.push_back(buttonInfo);
                }
                catch (const std::exception& e) {
                    ErrorsManager::addError("Social Buttons: Failed to allocate memory or process button at index " + std::to_string(i) + ": " + std::string(e.what()), ErrorsManager::Error);
                    return false;
                }
            }
        }
        catch (const nlohmann::json::parse_error& e) {
            ErrorsManager::addError("Social Buttons: Failed to parse JSON. Parsing error: " + std::string(e.what()), ErrorsManager::Error);
            return false;
        }
        catch (const nlohmann::json::type_error& e) {
            ErrorsManager::addError("Social Buttons: Type error while processing JSON: " + std::string(e.what()), ErrorsManager::Error);
            return false;
        }
        catch (const std::exception& e) {
            ErrorsManager::addError("Social Buttons: Unexpected exception: " + std::string(e.what()), ErrorsManager::Error);
            return false;
        }

        return true;
    }

	SocialButtonsManager() {};
public:
	void onMenuLayer(MenuLayer* layer) {
		if (!isOk) return;

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
			btnSprite->setContentSize({31.5, 31.5});
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
		ErrorsManager::onMenuLayer(this);
		return true;
	}
};