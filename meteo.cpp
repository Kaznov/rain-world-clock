#include "meteo.hpp"

#include <ctime>

MeteoData meteo_data;

enum WeatherCode {
	ClearSkies = 1000,
	PartlyCloudy = 1003,
	Cloudy = 1006,
	Overcast = 1009,
	Mist = 1030,
	PatchyRainPossible = 1063,
	PatchySnowPossible = 1066,
	PatchySleetPossible = 1069,
	PatchyFreezingDrizzlePossible = 1072,
	ThunderyOutbreaksPossible = 1087,
	BlowingSnow = 1114,
	Blizzard = 1117,
	Fog = 1135,
	FreezingFog = 1147,
	PatchyLightDrizzle = 1150,
	LightDrizzle = 1153,
	FreezingDrizzle = 1168,
	HeavyFreezingDrizzle = 1171,
	PatchyLightRain = 1180,
	LightRain = 1183,
	ModerateRainAtTimes = 1186,
	ModerateRain = 1189,
	HeavyRainAtTimes = 1192,
	HeavyRain = 1195,
	LightFreezingRain = 1198,
	ModerateOrHeavyFreezingRain = 1201,
	LightSleet = 1204,
	ModerateOrHeavySleet = 1207,
	PatchyLightSnow = 1210,
	LightSnow = 1213,
	PatchyModerateSnow = 1216,
	ModerateSnow = 1219,
	PatchyHeavySnow = 1222,
	HeavySnow = 1225,
	IcePellets = 1237,
	LightRainShower = 1240,
	ModerateOrHeavyRainShower = 1243,
	TorrentialRainShower = 1246,
	LightSleetShowers = 1249,
	ModerateOrHeavySleetShowers = 1252,
	LightSnowShowers = 1255,
	ModerateOrHeavySnowShowers = 1258,
	LightShowersOfIcePellets = 1261,
	ModerateOrHeavyShowersOfIcePellets = 1264,
	PatchyLightRainWithThunder = 1273,
	ModerateOrHeavyRainWithThunder = 1276,
	PatchyLightSnowWithThunder = 1279,
	ModerateOrHeavySnowWithThunder = 1282,
};

static int getMoonIllumination(time_t timestamp) {
    static constexpr time_t FULL_MOON_TIMESTAMP = 1860900; // 22-01-1970 12:55:00
    static constexpr time_t SECONDS_EVERY_MOON  = 2551442; // 29.5306 days

    auto llabs = [](const long long x) { return x < 0 ? -x : x; };
    auto get_percentage_of = [](const long long x, const long long all) {
        return (x * 100 + all/2) / all;
    };

    const time_t moon_state = (timestamp - FULL_MOON_TIMESTAMP) % SECONDS_EVERY_MOON;
    // 0       - full moon
    // 1275721 - new moon
    // 2551442 - full moon

    const int illumination = (int)(llabs(moon_state - SECONDS_EVERY_MOON / 2));
    return get_percentage_of(illumination, SECONDS_EVERY_MOON / 2);
}

const char* getWeatherDayIconName(WeatherCode code) {
    switch (code)
    {
    case ClearSkies:
        return "0d_wi-day-sunny.bmp";
	case PartlyCloudy:
        return "1d_wi-day-cloudy.bmp";
    case Cloudy:
        return "2d_wi-cloud.bmp";
	case Overcast:
        return "3d_wi-cloudy.bmp";
	case Mist:
        return "wi-day-fog.bmp";
	case PatchyRainPossible:
        return "wi-day-rain.bmp";
	case PatchySnowPossible:
        return "wi-day-snow.bmp";
	case PatchySleetPossible:
	case PatchyFreezingDrizzlePossible:
        return "wi-day-sleet.bmp";
	case ThunderyOutbreaksPossible:
        "wi-day-lightning.bmp";
	case BlowingSnow:
	case Blizzard:
        "wi-snow-wind.bmp";
	case Fog:
	case FreezingFog:
        return "wi-fog.bmp";
	case PatchyLightDrizzle:
	case LightDrizzle:
        return "wi-day-sleet.bmp";
	case FreezingDrizzle:
	case HeavyFreezingDrizzle:
        return "wi-sleet.bmp";
    case PatchyLightRain:
	case LightRain:
        return "wi-day-showers.bmp";
	case ModerateRainAtTimes:
	case ModerateRain:
        return "wi-day-rain.bmp";
    case HeavyRainAtTimes:
	case HeavyRain:
        return "wi-rain.bmp";
	case LightFreezingRain:
	case ModerateOrHeavyFreezingRain:
	case LightSleet:
        return "wi-day-sleet.bmp";
	case ModerateOrHeavySleet:
        return "wi-sleet.bmp";
    case PatchyLightSnow:
	case LightSnow:
	case PatchyModerateSnow:
        return "wi-day-snow.bmp";
	case ModerateSnow:
	case PatchyHeavySnow:
	case HeavySnow:
        return "wi-snow.bmp";
	case IcePellets:
        return "wi-hail.bmp";
	case LightRainShower:
        return "wi-day-showers.bmp";
	case ModerateOrHeavyRainShower:
        return "wi-showers.bmp";
	case TorrentialRainShower:
        return "wi-showers.bmp";
	case LightSleetShowers:
        return "wi-day-sleet.bmp";
	case ModerateOrHeavySleetShowers:
        return "wi-sleet.bmp";
	case LightSnowShowers:
        return "wi-day-snow.bmp";
	case ModerateOrHeavySnowShowers:
        return "wi-snow.bmp";
	case LightShowersOfIcePellets:
        return "wi-day-hail.bmp";
	case ModerateOrHeavyShowersOfIcePellets:
        return "wi-hail.bmp";
	case PatchyLightRainWithThunder:
        return "wi-storm-showers.bmp";
	case ModerateOrHeavyRainWithThunder:
        return "wi-thunderstorm.bmp";
	case PatchyLightSnowWithThunder:
        return "wi-day-snow-thunderstorm.bmp";
	case ModerateOrHeavySnowWithThunder:
        return "wi-snow-thunderstorm.bmp";
    default:
        return "wi-na.bmp";
    }
}

const char* getWeatherNightIconName(WeatherCode code, bool moon_visible) {
    switch (code)
    {
    case ClearSkies:
        return moon_visible ? "0d_wi-night-clear.bmp" : "wi-stars.bmp";
	case PartlyCloudy:
        return "wi-night-alt-cloudy.bmp";
    case Cloudy:
        return "2d_wi-cloud.bmp";
	case Overcast:
        return "3d_wi-cloudy.bmp";
	case Mist:
        return "wi-night-fog.bmp";
	case PatchyRainPossible:
        return "wi-night-alt-rain.bmp";
	case PatchySnowPossible:
        return "wi-night-alt-snow.bmp";
	case PatchySleetPossible:
	case PatchyFreezingDrizzlePossible:
        return "wi-night-alt-sleet.bmp";
	case ThunderyOutbreaksPossible:
        "wi-night-alt-lightning.bmp";
	case BlowingSnow:
	case Blizzard:
        "wi-snow-wind.bmp";
	case Fog:
	case FreezingFog:
        return "wi-fog.bmp";
	case PatchyLightDrizzle:
	case LightDrizzle:
        return "wi-night-alt-sleet.bmp";
	case FreezingDrizzle:
	case HeavyFreezingDrizzle:
        return "wi-sleet.bmp";
    case PatchyLightRain:
	case LightRain:
        return "wi-night-alt-showers.bmp";
	case ModerateRainAtTimes:
	case ModerateRain:
        return "wi-night-alt-rain.bmp";
    case HeavyRainAtTimes:
	case HeavyRain:
        return "wi-rain.bmp";
	case LightFreezingRain:
	case ModerateOrHeavyFreezingRain:
	case LightSleet:
        return "wi-night-alt-sleet.bmp";
	case ModerateOrHeavySleet:
        return "wi-sleet.bmp";
    case PatchyLightSnow:
	case LightSnow:
	case PatchyModerateSnow:
        return "wi-night-alt-snow.bmp";
	case ModerateSnow:
	case PatchyHeavySnow:
	case HeavySnow:
        return "wi-snow.bmp";
	case IcePellets:
        return "wi-hail.bmp";
	case LightRainShower:
        return "wi-night-alt-showers.bmp";
	case ModerateOrHeavyRainShower:
        return "wi-showers.bmp";
	case TorrentialRainShower:
        return "wi-showers.bmp";
	case LightSleetShowers:
        return "wi-night-alt-sleet.bmp";
	case ModerateOrHeavySleetShowers:
        return "wi-sleet.bmp";
	case LightSnowShowers:
        return "wi-night-alt-snow.bmp";
	case ModerateOrHeavySnowShowers:
        return "wi-snow.bmp";
	case LightShowersOfIcePellets:
        return "wi-night-alt-hail.bmp";
	case ModerateOrHeavyShowersOfIcePellets:
        return "wi-hail.bmp";
	case PatchyLightRainWithThunder:
        return "wi-storm-showers.bmp";
	case ModerateOrHeavyRainWithThunder:
        return "wi-thunderstorm.bmp";
	case PatchyLightSnowWithThunder:
        return "wi-night-alt-snow-thunderstorm.bmp";
	case ModerateOrHeavySnowWithThunder:
        return "wi-snow-thunderstorm.bmp";
    default:
        return "wi-na.bmp";
    }
}
