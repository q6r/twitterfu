#include "Filters.h"

Filters::Filters() {
}

Filters::~Filters() {
}

void Filters::setProfilePicture(bool n) {
        profilePicture = n;
}

void Filters::setDescription(bool n) {
        description = n;
}

void Filters::setProtectedProfile(bool n) {
        protectedProfile = n;
}

void Filters::setFollowRatio(bool n) {
        followRatio = n;
}

void Filters::setNearTimezone(bool n) {
        nearTimezone = n;
}

bool Filters::getProfilePicture() {
        return profilePicture;
}

bool Filters::getDescription() {
        return description;
}

bool Filters::getProtectedProfile() {
        return protectedProfile;
}

bool Filters::getFollowRatio() {
        return followRatio;
}

bool Filters::getNearTimezone() {
        return nearTimezone;
}

bool Filters::mainfilter(User * user, string userid)
{
	string resultXML, temp_following, temp_followers, timezone;
	long double following, followers, result;
	int prediction = 0;
	int total = 0;

	if (user->twitterObj.userGet(userid, true) == false)
		return false;

	user->twitterObj.getLastWebResponse(resultXML);

	// get user following, and followers
	if (lastResponse(user, "user.followers_count", temp_followers) == false)
		return false;
	if (lastResponse(user, "user.friends_count", temp_following) == false)
		return false;
	stringstream sa(temp_following);
	stringstream sb(temp_followers);
	sa >> following;
	sb >> followers;

	/* rule #1      : Following more than followers or a ratio of 75% */
	if (Filters::getFollowRatio() == true) {
		if (following >= followers) {
			prediction++;
		} else {
			result = (following / followers) * 100;
			if (result >= 75) {
				prediction++;
			}
		}
		total++;
	}

	/* rule #2      : User not protected */
	if (Filters::getProtectedProfile() == true) {
		string protect;
		if (lastResponse(user, "user.protected", protect) == false)
			return false;
		if (protect == "false") {
			prediction++;
		}
		total++;
	}

	/* rule #3      : Has profile image */
	if (Filters::getProfilePicture() == true) {
		string profile_image;
		if (lastResponse
		    (user, "user.profile_image_url", profile_image) == false)
			return false;
		if (!profile_image.empty()) {
			prediction++;
		}
		total++;
	}

	/* rule #4      : Has description */
	if (Filters::getDescription() == true) {
		string description;
		if (lastResponse(user, "user.description", description) ==
		    false)
			return false;
		if (!description.empty()) {
			prediction++;
		}
		total++;
	}

	/* rule #5      : Predict by near by timezone of -4,+4
	 * ignore anyone who doesn't have a timezone
	 */
	if (Filters::getNearTimezone() == true) {
		if (lastResponse(user, "user.time_zone", timezone) == false) {
			return false;
		}
		// if he or us don't have timezones then false;
		if (timezone.empty() || user->getTimezone().empty()) {
			return false;
		}
		if (Filters::predict_timezone(user, timezone) == true)
			prediction++;
		total++;
	}
	// if prediction is not 100% then ignore user
	if (total != 0) {
		if (prediction / total != 1) {
			return false;
		}
	}

	return true;
}

void Filters::filter_list()
{
	int opt;

	cout << "Select one to toggle it" << endl;
	do {
		cout << "1) Have a profile picture : " << boolalpha <<
		    Filters::getProfilePicture() << endl;
		cout << "2) Have description       : " << boolalpha <<
		    Filters::getDescription() << endl;
		cout << "3) Protected profile      : " << boolalpha <<
		    Filters::getProtectedProfile() << endl;
		cout << "4) Following ratio        : " << boolalpha <<
		    Filters::getFollowRatio() << endl;
		cout << "5) Near by -4,+4 timezones: " << boolalpha <<
		    Filters::getNearTimezone() << endl;
		cout << "6) return" << endl;

		opt = optionSelect();
		switch (opt) {
		case 1:
			Filters::setProfilePicture(!Filters::getProfilePicture());
			break;
		case 2:
			Filters::setDescription(!Filters::getDescription());
			break;
		case 3:
			Filters::setProtectedProfile(!Filters::getProtectedProfile());
			break;
		case 4:
			Filters::setFollowRatio(!Filters::getFollowRatio());
			break;
		case 5:
			Filters::setNearTimezone(!Filters::getNearTimezone());
			break;
		case 6:	// return
			break;
		default:
			break;
		}
	}
	while (opt != 6);
}

bool Filters::predict_timezone(User * user, string timezone)
{
	size_t timezoneAt;
	vector < string > tzs;
	tzs.push_back("International Date Line West");
	tzs.push_back("Midway Island");
	tzs.push_back("American Samoa");
	tzs.push_back("Hawaii");
	tzs.push_back("Alaska");
	tzs.push_back("Pacific Time (US & Canada)");
	tzs.push_back("Tijuana");
	tzs.push_back("Mountain Time (US & Canada)");
	tzs.push_back("Arizona");
	tzs.push_back("Chihuahua");
	tzs.push_back("Mazatlan");
	tzs.push_back("Central Time (US & Canada)");
	tzs.push_back("Saskatchewan");
	tzs.push_back("Guadalajara");
	tzs.push_back("Mexico City");
	tzs.push_back("Monterrey");
	tzs.push_back("Central America");
	tzs.push_back("Eastern Time (US & Canada)");
	tzs.push_back("Indiana (East)");
	tzs.push_back("Bogota");
	tzs.push_back("Lima");
	tzs.push_back("Quito");
	tzs.push_back("Atlantic Time (Canada)");
	tzs.push_back("Caracas");
	tzs.push_back("La Paz");
	tzs.push_back("Santiago");
	tzs.push_back("Newfoundland");
	tzs.push_back("Brasilia");
	tzs.push_back("Buenos Aires");
	tzs.push_back("Georgetown");
	tzs.push_back("Greenland");
	tzs.push_back("Mid-Atlantic");
	tzs.push_back("Azores");
	tzs.push_back("Cape Verde Is.");
	tzs.push_back("Dublin");
	tzs.push_back("Edinburgh");
	tzs.push_back("Lisbon");
	tzs.push_back("London");
	tzs.push_back("Casablanca");
	tzs.push_back("Monrovia");
	tzs.push_back("UTC");
	tzs.push_back("Belgrade");
	tzs.push_back("Bratislava");
	tzs.push_back("Budapest");
	tzs.push_back("Ljubljana");
	tzs.push_back("Prague");
	tzs.push_back("Sarajevo");
	tzs.push_back("Skopje");
	tzs.push_back("Warsaw");
	tzs.push_back("Zagreb");
	tzs.push_back("Brussels");
	tzs.push_back("Copenhagen");
	tzs.push_back("Madrid");
	tzs.push_back("Paris");
	tzs.push_back("Amsterdam");
	tzs.push_back("Berlin");
	tzs.push_back("Bern");
	tzs.push_back("Rome");
	tzs.push_back("Stockholm");
	tzs.push_back("Vienna");
	tzs.push_back("West Central Africa");
	tzs.push_back("Bucharest");
	tzs.push_back("Cairo");
	tzs.push_back("Helsinki");
	tzs.push_back("Kyiv");
	tzs.push_back("Riga");
	tzs.push_back("Sofia");
	tzs.push_back("Tallinn");
	tzs.push_back("Vilnius");
	tzs.push_back("Athens");
	tzs.push_back("Istanbul");
	tzs.push_back("Minsk");
	tzs.push_back("Jerusalem");
	tzs.push_back("Harare");
	tzs.push_back("Pretoria");
	tzs.push_back("Moscow");
	tzs.push_back("St. Petersburg");
	tzs.push_back("Volgograd");
	tzs.push_back("Kuwait");
	tzs.push_back("Riyadh");
	tzs.push_back("Nairobi");
	tzs.push_back("Baghdad");
	tzs.push_back("Tehran");
	tzs.push_back("Abu Dhabi");
	tzs.push_back("Muscat");
	tzs.push_back("Baku");
	tzs.push_back("Tbilisi");
	tzs.push_back("Yerevan");
	tzs.push_back("Kabul");
	tzs.push_back("Ekaterinburg");
	tzs.push_back("Islamabad");
	tzs.push_back("Karachi");
	tzs.push_back("Tashkent");
	tzs.push_back("Chennai");
	tzs.push_back("Kolkata");
	tzs.push_back("Mumbai");
	tzs.push_back("New Delhi");
	tzs.push_back("Kathmandu");
	tzs.push_back("Astana");
	tzs.push_back("Dhaka");
	tzs.push_back("Sri Jayawardenepura");
	tzs.push_back("Almaty");
	tzs.push_back("Novosibirsk");
	tzs.push_back("Rangoon");
	tzs.push_back("Bangkok");
	tzs.push_back("Hanoi");
	tzs.push_back("Jakarta");
	tzs.push_back("Krasnoyarsk");
	tzs.push_back("Beijing");
	tzs.push_back("Chongqing");
	tzs.push_back("Hong Kong");
	tzs.push_back("Urumqi");
	tzs.push_back("Kuala Lumpur");
	tzs.push_back("Singapore");
	tzs.push_back("Taipei");
	tzs.push_back("Perth");
	tzs.push_back("Irkutsk");
	tzs.push_back("Ulaan Bataar");
	tzs.push_back("Seoul");
	tzs.push_back("Osaka");
	tzs.push_back("Sapporo");
	tzs.push_back("Tokyo");
	tzs.push_back("Yakutsk");
	tzs.push_back("Darwin");
	tzs.push_back("Adelaide");
	tzs.push_back("Canberra");
	tzs.push_back("Melbourne");
	tzs.push_back("Sydney");
	tzs.push_back("Brisbane");
	tzs.push_back("Hobart");
	tzs.push_back("Vladivostok");
	tzs.push_back("Guam");
	tzs.push_back("Port Moresby");
	tzs.push_back("Magadan");
	tzs.push_back("Solomon Is.");
	tzs.push_back("New Caledonia");
	tzs.push_back("Fiji");
	tzs.push_back("Kamchatka");
	tzs.push_back("Marshall Is.");
	tzs.push_back("Auckland");
	tzs.push_back("Wellington");
	tzs.push_back("Nuku'alofa");
	tzs.push_back("Tokelau Is.");
	tzs.push_back("Samoa");

	// get the index of our timezone in the
	// vector
	vector < string >::iterator it =
	    find(tzs.begin(), tzs.end(), user->getTimezone());
	if (it != tzs.end()) {
		timezoneAt = it - tzs.begin();
	} else {		// user->timezone is invalid timezone 'or not defined'
		return false;
	}

	// we take 4 before it and 4 after our timezone
	// and check if the user to follow timezone is in that
	// range
	for (size_t i = timezoneAt - 5;
	     i < timezoneAt + 5 && i >= 0 && i < tzs.size(); i++) {
		if (timezone == tzs.at(i))
			return true;
	}

	return false;
}