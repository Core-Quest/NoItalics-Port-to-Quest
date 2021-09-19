#include "main.hpp"

#include "beatsaber-hook/shared/utils/utils.h"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "modloader/shared/modloader.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp" 
#include "beatsaber-hook/shared/utils/typedefs.h"
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "TMPro/TextMeshPro.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "HMUI/ImageView.hpp"



#define PATH "/sdcard/Android/data/com.beatgames.beatsaber/files/logdump-"
#define EXT ".txt"

static ModInfo modInfo;


Configuration& getConfig() {
    static Configuration config(modInfo);
    return config;
}

Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
} 

void write_info(FILE* fp, std::string str) {
    getLogger().debug("%s", str.data());
    fwrite((str + "\n").data(), str.length() + 1, 1, fp);
}

bool unSkewImages = true;
bool removeItalics = true;

int getFontStyleValue(Il2CppObject* self){
	int fontStyle = CRASH_UNLESS(il2cpp_utils::GetPropertyValue<int>(self, "fontStyle"));
	if (fontStyle & (1 << 1)) {
		return fontStyle - 2;
	}
	else {
		return -1;
	}
}

MAKE_HOOK_MATCH(TextMeshPro_GenerateTextMesh, &::TMPro::TextMeshPro::GenerateTextMesh, void, TMPro::TextMeshPro* self) {
	if (!removeItalics) {
		TextMeshPro_GenerateTextMesh(self);
		return;
	}
	int value = getFontStyleValue(self);
	if (value != -1) CRASH_UNLESS(il2cpp_utils::SetPropertyValue(self, "fontStyle", value));
	TextMeshPro_GenerateTextMesh(self);
}

MAKE_HOOK_MATCH(TextMeshProUGUI_GenerateTextMesh, &::TMPro::TextMeshProUGUI::GenerateTextMesh, void, TMPro::TextMeshProUGUI* self) {
	if (!removeItalics) {
		TextMeshProUGUI_GenerateTextMesh(self);
		return;
	}
	int value = getFontStyleValue(self);
	if (value != -1) CRASH_UNLESS(il2cpp_utils::SetPropertyValue(self, "fontStyle", value));
	TextMeshProUGUI_GenerateTextMesh(self);
}

MAKE_HOOK_MATCH(ImageView_OnEnable, &HMUI::ImageView::OnEnable, void, HMUI::ImageView* self) {
	if (!unSkewImages) {
		ImageView_OnEnable(self);
		return;
	}
	float skew = 0.0f;
	il2cpp_utils::SetFieldValue(self, "_skew", skew);
	ImageView_OnEnable(self);
}


void resetconfig() {

	getConfig().config.RemoveAllMembers();
	getConfig().config.SetObject();
	auto& allocator = getConfig().config.GetAllocator();

	getConfig().config.AddMember("unSkewImages", unSkewImages, allocator);
	getConfig().config.AddMember("removeItalics", removeItalics, allocator);

	getConfig().Write();
}


extern "C" void setup(ModInfo& info) {
    info.id = "NoItalics";
    info.version = "1.0.0";
	modInfo = info;


    getLogger().info("Completed setup!");
    getLogger().info("Modloader name: %s", Modloader::getInfo().name.c_str());
	getConfig().Load();
	const auto& conf = getConfig().config;

	bool resetConfig = false;

	auto unSkewImagesitr = conf.FindMember("unSkewImages");
	if (unSkewImagesitr != conf.MemberEnd()) {
		unSkewImages = unSkewImagesitr->value.GetBool();
	}else { resetConfig = true; }
	auto removeItalicsitr = conf.FindMember("removeItalics");
	if (removeItalicsitr != conf.MemberEnd()) {
		removeItalics = removeItalicsitr->value.GetBool();
	}else { resetConfig = true; }
	
	if (resetConfig) {
		resetconfig();
	}

}




// This function is called when the mod is loaded for the first time, immediately after il2cpp_init.
extern "C" void load() {
    getLogger().debug("Installing NoItalics!");
		
	INSTALL_HOOK(getLogger(), ImageView_OnEnable);
	INSTALL_HOOK(getLogger(), TextMeshPro_GenerateTextMesh);
	INSTALL_HOOK(getLogger(), TextMeshProUGUI_GenerateTextMesh);


	getLogger().debug("Installed NoItalics!");
    getLogger().info("initialized: %s", il2cpp_functions::initialized ? "true" : "false");
}