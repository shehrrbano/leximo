#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <map>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cmath>
#include <algorithm>

using namespace std;

// Forward declarations
class UserProgress;
class GrammarLesson;
class ReadingComprehension;

// Achievement System
class Achievement {
public:
    string name;
    string description;
    bool unlocked;
    int pointsReward;

    Achievement(string n, string desc, int points)
        : name(n), description(desc), unlocked(false), pointsReward(points) {}
};

// User Level System
class UserLevel {
public:
    int level;
    int currentExp;
    int expToNextLevel;
    int totalPoints;
    int streak;
    string rank;

    UserLevel() : level(1), currentExp(0), expToNextLevel(100),
                 totalPoints(0), streak(0), rank("Beginner") {}

    void addExperience(int exp) {
        currentExp += exp;
        while (currentExp >= expToNextLevel) {
            levelUp();
        }
    }

    void levelUp() {
        level++;
        currentExp -= expToNextLevel;
        expToNextLevel = 100 * level; // Increases exp needed for each level
        updateRank();
    }

    void updateRank() {
        if (level < 5) rank = "Beginner";
        else if (level < 10) rank = "Intermediate";
        else if (level < 15) rank = "Advanced";
        else if (level < 20) rank = "Expert";
        else rank = "Master";
    }

    void incrementStreak() {
        streak++;
        if (streak % 7 == 0) { // Weekly streak bonus
            addExperience(50);
            totalPoints += 100;
        }
    }

    void resetStreak() {
        streak = 0;
    }
};

// Skill Tree Node
class SkillNode {
public:
    string name;
    string description;
    bool unlocked;
    vector<SkillNode*> prerequisites;
    vector<string> content;
    string difficulty;

    SkillNode(string n, string desc, string diff)
        : name(n), description(desc), unlocked(false), difficulty(diff) {}

    bool canUnlock() {
        if (prerequisites.empty()) return true;
        return all_of(prerequisites.begin(), prerequisites.end(),
                     [](SkillNode* pre) { return pre->unlocked; });
    }
};

// Extended Word class with pronunciation
class Word {
public:
    string word;
    string meaning;
    string pronunciation;
    string partOfSpeech;
    vector<string> examples;
    vector<string> translations;

    Word(string w, string m, string pron)
        : word(w), meaning(m), pronunciation(pron) {}

    void addExample(const string& example) {
        examples.push_back(example);
    }

    void addTranslation(const string& translation) {
        translations.push_back(translation);
    }

    void display() const {
        cout << "Word: " << word << endl;
        cout << "Pronunciation: " << pronunciation << endl;
        cout << "Meaning: " << meaning << endl;
        if (!partOfSpeech.empty())
            cout << "Part of Speech: " << partOfSpeech << endl;

        if (!examples.empty()) {
            cout << "Examples:" << endl;
            for (const auto& example : examples) {
                cout << "- " << example << endl;
            }
        }

        if (!translations.empty()) {
            cout << "Translations:" << endl;
            for (const auto& trans : translations) {
                cout << "- " << trans << endl;
            }
        }
    }
};

class UserProgress {
public:
    string username;
    map<string, int> wordProgress;
    map<string, chrono::system_clock::time_point> nextReview;
    vector<Achievement> achievements;
    vector<SkillNode*> unlockedSkills;
    int dailyGoal;
    int wordsLearnedToday;
    vector<string> completedLessons;
    chrono::system_clock::time_point lastLoginTime;
    UserLevel level;

    UserProgress(const string& name)
        : username(name),
          dailyGoal(10),
          wordsLearnedToday(0),
          lastLoginTime(chrono::system_clock::now()) {
        initializeAchievements();
    }

    void updateWordProgress(const string& word) {
        wordProgress[word]++;
        calculateNextReview(word);
    }

    void calculateNextReview(const string& word) {
        int level = wordProgress[word];
        int hours = pow(2, level);  // Exponential spacing
        auto now = chrono::system_clock::now();
        nextReview[word] = now + chrono::hours(hours);
    }

    bool isWordDueForReview(const string& word) {
        if (nextReview.find(word) == nextReview.end()) return true;
        return chrono::system_clock::now() >= nextReview[word];
    }

    void saveToDisk() {
        ofstream file(username + "_progress.txt");

        // Save basic stats
        file << dailyGoal << endl;
        file << wordsLearnedToday << endl;
        file << level.level << endl;
        file << level.currentExp << endl;
        file << level.streak << endl;

        // Save the last login timestamp
        auto timestamp = chrono::system_clock::to_time_t(lastLoginTime);
        file << timestamp << endl;

        // Save word progress
        file << wordProgress.size() << endl;
        for (const auto& pair : wordProgress) {
            file << pair.first << "," << pair.second << endl;
        }

        // Save completed lessons
        file << completedLessons.size() << endl;
        for (const auto& lesson : completedLessons) {
            file << lesson << endl;
        }
    }

    void loadFromDisk() {
        ifstream file(username + "_progress.txt");
        if (!file.is_open()) return;

        // Load basic stats
        file >> dailyGoal;
        file >> wordsLearnedToday;
        file >> level.level;
        file >> level.currentExp;
        file >> level.streak;

        // Load last login time
        time_t timestamp;
        file >> timestamp;
        lastLoginTime = chrono::system_clock::from_time_t(timestamp);

        // Skip the newline
        file.ignore();

        // Load word progress
        int wordCount;
        file >> wordCount;
        file.ignore(); // Skip newline

        for (int i = 0; i < wordCount; i++) {
            string line;
            getline(file, line);
            stringstream ss(line);
            string word;
            int progress;
            getline(ss, word, ',');
            ss >> progress;
            wordProgress[word] = progress;
        }

        // Load completed lessons
        int lessonCount;
        file >> lessonCount;
        file.ignore(); // Skip newline

        for (int i = 0; i < lessonCount; i++) {
            string lesson;
            getline(file, lesson);
            completedLessons.push_back(lesson);
        }

        // Update level rank after loading
        level.updateRank();
    }

    void initializeAchievements() {
        achievements.push_back(Achievement("First Steps", "Complete your first lesson", 50));
        achievements.push_back(Achievement("Word Master", "Learn 100 words", 100));
        achievements.push_back(Achievement("Grammar Expert", "Complete 10 grammar lessons", 150));
        achievements.push_back(Achievement("Dedicated Learner", "Maintain a 7-day streak", 200));
        achievements.push_back(Achievement("Polyglot", "Complete translations in multiple languages", 250));
    }

    void checkAchievements() {
        // Check for First Steps
        if (!achievements[0].unlocked && !completedLessons.empty()) {
            unlockAchievement(0);
        }
        // Check for Word Master
        if (!achievements[1].unlocked && wordProgress.size() >= 100) {
            unlockAchievement(1);
        }
        // Check for Grammar Expert
        if (!achievements[2].unlocked && completedLessons.size() >= 10) {
            unlockAchievement(2);
        }
        // Check for Dedicated Learner
        if (!achievements[3].unlocked && level.streak >= 7) {
            unlockAchievement(3);
        }
    }

    void unlockAchievement(int index) {
        if (!achievements[index].unlocked) {
            achievements[index].unlocked = true;
            level.addExperience(achievements[index].pointsReward);
            level.totalPoints += achievements[index].pointsReward;

            cout << "\n*** Achievement Unlocked! ***" << endl;
            cout << achievements[index].name << ": " << achievements[index].description << endl;
            cout << "Reward: " << achievements[index].pointsReward << " points!" << endl;
        }
    }

    void displayProgress() const {
        cout << "\n=== Progress Report for " << username << " ===" << endl;
        cout << "Level: " << level.level << " (" << level.rank << ")" << endl;
        cout << "Experience: " << level.currentExp << "/" << level.expToNextLevel << endl;
        cout << "Total Points: " << level.totalPoints << endl;
        cout << "Current Streak: " << level.streak << " days" << endl;
        cout << "\nWords Learned: " << wordProgress.size() << endl;
        cout << "Lessons Completed: " << completedLessons.size() << endl;

        cout << "\nAchievements:" << endl;
        for (const auto& achievement : achievements) {
            cout << (achievement.unlocked ? "[X] " : "[ ] ")
                 << achievement.name << " - " << achievement.description << endl;
        }
    }
};

// Grammar lesson structure
class GrammarLesson {
public:
    string title;
    string explanation;
    vector<pair<string, string>> examples;  // Example and its explanation
    vector<pair<string, string>> exercises; // Question and answer

    GrammarLesson(string t, string exp) : title(t), explanation(exp) {}

    void addExample(const string& example, const string& explanation) {
        examples.push_back(make_pair(example, explanation));
    }

    void addExercise(const string& question, const string& answer) {
        exercises.push_back(make_pair(question, answer));
    }

    void display() const {
        cout << "\n=== " << title << " ===" << endl;
        cout << "\nExplanation:\n" << explanation << "\n\n";
        cout << "Examples:\n";
        for (size_t i = 0; i < examples.size(); ++i) {
            cout << i + 1 << ". " << examples[i].first << endl;
            cout << "   Explanation: " << examples[i].second << endl;
        }
    }

    void practice() const {
        cout << "\nPractice Exercises:\n";
        for (size_t i = 0; i < exercises.size(); ++i) {
            cout << "\nQuestion " << i + 1 << ": " << exercises[i].first << endl;
            cout << "Your answer: ";
            string userAnswer;
            getline(cin, userAnswer);

            cout << "Correct answer: " << exercises[i].second << endl;
            if (userAnswer == exercises[i].second) {
                cout << "Correct!\n";
            } else {
                cout << "Keep practicing. The correct answer is: " << exercises[i].second << "\n";
            }
        }
    }
};

// Translation Exercise class
class TranslationExercise {
public:
    Word word;
    vector<string> options;
    string correctTranslation;
    string targetLanguage;

    TranslationExercise(const Word& w, const vector<string>& opt,
                       const string& correct, const string& lang)
        : word(w), options(opt), correctTranslation(correct), targetLanguage(lang) {}

    bool practice() {
        cout << "\nTranslate '" << word.word << "' to " << targetLanguage << ":" << endl;
        cout << "Meaning: " << word.meaning << endl;

        for (size_t i = 0; i < options.size(); ++i) {
            cout << i + 1 << ". " << options[i] << endl;
        }

        cout << "\nYour choice (1-" << options.size() << "): ";
        string input;
        getline(cin, input);

        if (isdigit(input[0])) {
            int choice = stoi(input) - 1;
            if (choice >= 0 && choice < options.size()) {
                bool correct = options[choice] == correctTranslation;
                if (correct) {
                    cout << "\nCorrect! Well done!" << endl;
                } else {
                    cout << "\nIncorrect. The correct translation is: "
                         << correctTranslation << endl;
                }
                return correct;
            }
        }
        return false;
    }
};

// Reading comprehension structure
class ReadingComprehension {
public:
    string title;
    string text;
    vector<pair<string, string>> questions;  // Question and answer
    string difficulty;

    ReadingComprehension(string t, string txt, string diff)
        : title(t), text(txt), difficulty(diff) {}

    void addQuestion(const string& question, const string& answer) {
        questions.push_back(make_pair(question, answer));
    }

    void display() const {
        cout << "\n=== " << title << " ===\n";
        cout << "Difficulty: " << difficulty << "\n\n";
        cout << text << "\n\n";
    }

    void practice() const {
        display();
        cout << "\nComprehension Questions:\n";
        int score = 0;

        for (size_t i = 0; i < questions.size(); ++i) {
            cout << "\nQuestion " << i + 1 << ": " << questions[i].first << endl;
            cout << "Your answer: ";
            string userAnswer;
            getline(cin, userAnswer);

            if (userAnswer == questions[i].second) {
                cout << "Correct!\n";
                score++;
            } else {
                cout << "The correct answer is: " << questions[i].second << "\n";
            }
        }

        cout << "\nYour score: " << score << "/" << questions.size() << endl;
    }
};

// Enhanced SubCategory class with spaced repetition
class SubCategory {
public:
    string name;
    string description;
    vector<pair<string, string>> words;
    vector<GrammarLesson> grammarLessons;

    SubCategory(string n, string desc) : name(n), description(desc) {}

    void addWord(const string& word, const string& meaning) {
        words.push_back(make_pair(word, meaning));
    }

    void addGrammarLesson(const GrammarLesson& lesson) {
        grammarLessons.push_back(lesson);
    }

    void displayWords(UserProgress& progress) const {
        cout << "=== " << name << " ===" << endl;
        cout << description << "\n\n";
        for (size_t i = 0; i < words.size(); ++i) {
            cout << i + 1 << ". " << words[i].first << endl;
            cout << "   Meaning: " << words[i].second << endl;

            // Show progress if word has been studied
            if (progress.wordProgress.find(words[i].first) != progress.wordProgress.end()) {
                cout << "   Progress: Level " << progress.wordProgress[words[i].first] << endl;
            }
            cout << endl;
        }
    }

    void studyWords(UserProgress& progress) const {
        vector<pair<string, string>> reviewWords;
        for (const auto& word : words) {
            if (progress.isWordDueForReview(word.first)) {
                reviewWords.push_back(word);
            }
        }

        if (reviewWords.empty()) {
            cout << "No words due for review in this category!" << endl;
            return;
        }

        cout << "\nTime to review some words!\n";
        for (const auto& word : reviewWords) {
            cout << "\nWord: " << word.first << endl;
            cout << "Do you know the meaning? (Press Enter to see)" << endl;
            cin.get();

            cout << "Meaning: " << word.second << endl;
            cout << "Did you know this? (y/n): ";
            string response;
            getline(cin, response);

            if (response == "y") {
                progress.updateWordProgress(word.first);
                progress.wordsLearnedToday++;
            }
        }
    }
};

class Category {
public:
    string name;
    string description;
    vector<SubCategory> subcategories;
    vector<ReadingComprehension> readingMaterials;

    Category(string name, string desc) : name(name), description(desc) {}

    void addSubCategory(const SubCategory& subcat) {
        subcategories.push_back(subcat);
    }

    void addReadingMaterial(const ReadingComprehension& material) {
        readingMaterials.push_back(material);
    }

    void displayReadingMaterials() const {
        cout << "\nReading Materials for " << name << ":\n\n";
        for (size_t i = 0; i < readingMaterials.size(); ++i) {
            cout << i + 1 << ". " << readingMaterials[i].title
                 << " (" << readingMaterials[i].difficulty << ")" << endl;
        }
    }
};

// Helper functions
void customSleep(int milliseconds) {
    clock_t start_time = clock();
    while (clock() < start_time + milliseconds * (CLOCKS_PER_SEC / 1000));
}

void typeMessage(const string& message, int delay = 50) {
    for (const char& ch : message) {
        cout << ch;
        cout.flush();
        customSleep(delay);
    }
    cout << endl;
}

void displayDailyProgress(const UserProgress& progress) {
    cout << "\n=== Daily Progress ===" << endl;
    cout << "Words learned today: " << progress.wordsLearnedToday << "/"
         << progress.dailyGoal << endl;
    cout << "Completed lessons: " << progress.completedLessons.size() << endl;

    // Display progress bar
    cout << "Progress: [";
    int percentage = (progress.wordsLearnedToday * 100) / progress.dailyGoal;
    for (int i = 0; i < 20; i++) {
        if (i < (percentage / 5)) cout << "=";
        else cout << " ";
    }
    cout << "] " << percentage << "%" << endl;
}

void populateGrammarLessons(vector<GrammarLesson>& lessons) {
    // Present Simple
    GrammarLesson presentSimple("Present Simple Tense",
        "Used for habits, repeated actions, and general truths.");
    presentSimple.addExample("I play tennis every Sunday.",
        "Regular habit or repeated action");
    presentSimple.addExample("The sun rises in the east.",
        "General truth");
    presentSimple.addExercise("The Earth ___ (rotate) around the sun.",
        "rotates");
    presentSimple.addExercise("She ___ (study) English every day.",
        "studies");
    lessons.push_back(presentSimple);

    // Present Continuous
    GrammarLesson presentContinuous("Present Continuous Tense",
        "Used for actions happening now or temporary actions.");
    presentContinuous.addExample("I am writing code right now.",
        "Action happening at the moment");
    presentContinuous.addExample("She is studying for her exam this week.",
        "Temporary action");
    presentContinuous.addExercise("They ___ (play) football at the moment.",
        "are playing");
    presentContinuous.addExercise("I ___ (learn) Japanese this year.",
        "am learning");
    lessons.push_back(presentContinuous);
}

void populateReadingMaterials(vector<ReadingComprehension>& materials) {
    // Beginner level story
    ReadingComprehension story1(
        "A Day in the Park",
        "Sarah went to the park on a sunny day. She saw many dogs playing "
        "with their owners. There was a big tree with birds singing in its "
        "branches. Sarah sat on a bench and read her favorite book. It was "
        "a perfect day for being outside.",
        "Beginner"
    );
    story1.addQuestion("Where did Sarah go?", "to the park");
    story1.addQuestion("What was the weather like?", "sunny");
    story1.addQuestion("What did Sarah do on the bench?", "read a book");
    materials.push_back(story1);

    // Intermediate level story
    ReadingComprehension story2(
        "The Mystery of the Missing Book",
        "John was sure he had left his history book on his desk last night. "
        "But when he woke up in the morning, it wasn't there. He searched his "
        "entire room, looked under the bed, and checked his backpack. Finally, "
        "he found the book in the kitchen. His little sister had borrowed it "
        "to look at the pictures.",
        "Intermediate"
    );
    story2.addQuestion("What was missing?", "history book");
    story2.addQuestion("Where did John finally find the book?", "in the kitchen");
    story2.addQuestion("Who had taken the book?", "his little sister");
    materials.push_back(story2);
}

void populateVocabulary(vector<Category>& vocabulary) {
    // Animals Category
    Category animals("Animals", "Learn about different types of animals");

    // Mammals Subcategory
    SubCategory mammals("Mammals", "Warm-blooded animals that give birth to live young");
    mammals.addWord("lion", "King of the jungle, a large wild cat");
    mammals.addWord("elephant", "Largest land mammal with a trunk");
    mammals.addWord("dolphin", "Intelligent aquatic mammal");

    // Add grammar lessons specific to animal vocabulary
    GrammarLesson animalArticles("Articles with Animal Names",
        "Learn when to use 'a', 'an', and 'the' with animal names.");
    animalArticles.addExample("I saw an elephant at the zoo.",
        "Use 'an' before vowel sounds");
    animalArticles.addExample("The lion is known as the king of the jungle.",
        "Use 'the' for specific references");
    mammals.addGrammarLesson(animalArticles);

    animals.addSubCategory(mammals);

    // Add reading materials for animals category
    ReadingComprehension animalStory(
        "A Day at the Zoo",
        "Yesterday, I visited the zoo with my family. We saw many amazing "
        "animals. The elephants were spraying water with their trunks. The "
        "monkeys were swinging from tree to tree. My favorite was watching "
        "the penguins swim underwater.",
        "Beginner"
    );
    animalStory.addQuestion("Where did the narrator go?", "the zoo");
    animalStory.addQuestion("What were the elephants doing?", "spraying water");
    animals.addReadingMaterial(animalStory);

    vocabulary.push_back(animals);

    Category professions("Professions", "Learn about different jobs and careers");

    SubCategory medicalProfessions("Medical Professions", "Healthcare-related careers");
    medicalProfessions.addWord("doctor", "Medical professional who treats patients");
    medicalProfessions.addWord("nurse", "Healthcare professional who cares for patients");
    medicalProfessions.addWord("surgeon", "Doctor who performs operations");
    medicalProfessions.addWord("pediatrician", "Doctor specializing in child healthcare");
    medicalProfessions.addWord("dentist", "Doctor specializing in oral health");
    medicalProfessions.addWord("pharmacist", "Professional who prepares and dispenses medicines");

    SubCategory techProfessions("Technology Professions", "Careers in technology and computing");
    techProfessions.addWord("programmer", "Person who writes computer code");
    techProfessions.addWord("engineer", "Professional who designs and builds things");
    techProfessions.addWord("analyst", "Person who analyzes data and systems");
    techProfessions.addWord("designer", "Person who creates visual and user experiences");
    techProfessions.addWord("developer", "Person who builds software applications");

    // Add grammar lessons for professions
    GrammarLesson professionArticles("Articles with Professions",
        "Learn how to use articles when talking about jobs");
    professionArticles.addExample("She is a doctor", "Use 'a' for general statements about professions");
    professionArticles.addExample("The doctor who treated me was kind", "Use 'the' for specific references");

    professions.addSubCategory(medicalProfessions);
    professions.addSubCategory(techProfessions);
    vocabulary.push_back(professions);

    // 3. Food and Cooking
    Category food("Food and Cooking", "Learn about different types of food and cooking terms");

    SubCategory fruits("Fruits", "Different types of fruits");
    fruits.addWord("apple", "Round fruit that can be red or green");
    fruits.addWord("banana", "Long yellow fruit");
    fruits.addWord("orange", "Round citrus fruit");
    fruits.addWord("grape", "Small, sweet fruit growing in clusters");
    fruits.addWord("mango", "Sweet tropical fruit");

    SubCategory vegetables("Vegetables", "Different types of vegetables");
    vegetables.addWord("carrot", "Orange root vegetable");
    vegetables.addWord("potato", "Starchy root vegetable");
    vegetables.addWord("tomato", "Red fruit used as a vegetable");
    vegetables.addWord("lettuce", "Leafy green vegetable");
    vegetables.addWord("cucumber", "Long green vegetable");

    food.addSubCategory(fruits);
    food.addSubCategory(vegetables);
    vocabulary.push_back(food);

    // 4. Technology
    Category technology("Technology", "Modern technology and digital terms");

    SubCategory devices("Devices", "Electronic devices and gadgets");
    devices.addWord("smartphone", "Mobile phone with advanced computing capability");
    devices.addWord("laptop", "Portable computer");
    devices.addWord("tablet", "Portable touchscreen device");
    devices.addWord("smartwatch", "Wearable computing device");

    SubCategory software("Software", "Computer programs and applications");
    software.addWord("app", "Application program for devices");
    software.addWord("browser", "Program for accessing the internet");
    software.addWord("operating system", "Basic software managing computer hardware");
    software.addWord("antivirus", "Program protecting against malicious software");

    technology.addSubCategory(devices);
    technology.addSubCategory(software);
    vocabulary.push_back(technology);

    // 5. Travel
    Category travel("Travel", "Vocabulary related to travel and transportation");

    SubCategory transportation("Transportation", "Different modes of transport");
    transportation.addWord("airplane", "Aircraft for air travel");
    transportation.addWord("train", "Rail transport vehicle");
    transportation.addWord("bus", "Large road vehicle for passengers");
    transportation.addWord("taxi", "Car service with driver");

    SubCategory accommodation("Accommodation", "Places to stay while traveling");
    accommodation.addWord("hotel", "Establishment providing paid lodging");
    accommodation.addWord("hostel", "Budget-friendly shared accommodation");
    accommodation.addWord("resort", "Place for relaxation and recreation");
    accommodation.addWord("motel", "Roadside hotel for motorists");

    travel.addSubCategory(transportation);
    travel.addSubCategory(accommodation);
    vocabulary.push_back(travel);

    // 6. Sports and Recreation
    Category sports("Sports", "Different types of sports and recreational activities");

    SubCategory teamSports("Team Sports", "Sports played in teams");
    teamSports.addWord("football", "Sport played with a ball between two teams");
    teamSports.addWord("basketball", "Sport played with a ball and hoops");
    teamSports.addWord("volleyball", "Sport played over a net");

    SubCategory individualSports("Individual Sports", "Sports played individually");
    individualSports.addWord("swimming", "Sport of moving through water");
    individualSports.addWord("tennis", "Sport played with rackets");
    individualSports.addWord("golf", "Sport played with clubs and balls");

    sports.addSubCategory(teamSports);
    sports.addSubCategory(individualSports);
    vocabulary.push_back(sports);

    // Add reading materials for each category
    ReadingComprehension professionStory(
        "A Day in the Hospital",
        "Dr. Sarah Thompson walked through the busy hospital corridors. She had been a surgeon for ten years now. "
        "Today, she was working with a new nurse, James, who had just graduated from nursing school. "
        "In the emergency room, they treated several patients together. James learned a lot from watching "
        "Dr. Thompson work with such care and precision. At the end of their shift, Dr. Thompson thanked James "
        "for his help and encouraged him to keep learning.",
        "Intermediate"
    );
    professionStory.addQuestion("What is Dr. Thompson's profession?", "surgeon");
    professionStory.addQuestion("How long had she been a surgeon?", "ten years");
    professionStory.addQuestion("Who was the new staff member?", "James");
    professions.addReadingMaterial(professionStory);

    ReadingComprehension foodStory(
        "Cooking with Grandma",
        "Every Sunday, I cook with my grandmother. She teaches me how to make traditional recipes. "
        "Today, we're making her famous vegetable soup. First, we chop carrots, potatoes, and tomatoes. "
        "Then, we add them to boiling water with some herbs. Grandma says the secret is to cook it slowly. "
        "While we wait, she tells me stories about how she learned these recipes from her own grandmother.",
        "Beginner"
    );
    foodStory.addQuestion("When do they cook together?", "Every Sunday");
    foodStory.addQuestion("What are they cooking?", "vegetable soup");
    foodStory.addQuestion("What is the secret?", "to cook it slowly");
    food.addReadingMaterial(foodStory);

    ReadingComprehension techStory(
        "The New Smartphone",
        "Emma just bought her first smartphone. It was very different from her old phone. "
        "She spent the morning downloading apps and setting up her email. The phone had "
        "a large touchscreen and a great camera. Emma's favorite feature was the voice assistant, "
        "which could help her set reminders and find directions. She was excited to learn more "
        "about all the things her new phone could do.",
        "Beginner"
    );
    techStory.addQuestion("What did Emma buy?", "smartphone");
    techStory.addQuestion("What was her favorite feature?", "voice assistant");
    techStory.addQuestion("What did she do in the morning?", "downloading apps");
    technology.addReadingMaterial(techStory);

    ReadingComprehension travelStory(
        "An Adventure Abroad",
        "Maria was planning her first trip abroad. She booked a flight to Paris and reserved "
        "a room in a small hotel near the city center. The flight was eight hours long, but "
        "she passed the time by watching movies and reading. When she arrived, she took a taxi "
        "from the airport to her hotel. The driver was friendly and pointed out famous landmarks "
        "along the way. Maria couldn't wait to start exploring the city.",
        "Intermediate"
    );
    travelStory.addQuestion("Where did Maria travel to?", "Paris");
    travelStory.addQuestion("How long was the flight?", "eight hours");
    travelStory.addQuestion("How did she get to her hotel?", "taxi");
    travel.addReadingMaterial(travelStory);

    ReadingComprehension sportsStory(
        "The Big Game",
        "The basketball team had been practicing for months. Today was the final game of the season. "
        "The players warmed up on the court while fans filled the stands. The game was close, with both "
        "teams playing well. In the last minute, Tom scored a three-pointer to win the game. "
        "Everyone cheered as the team celebrated their victory.",
        "Beginner"
    );
    sportsStory.addQuestion("What sport were they playing?", "basketball");
    sportsStory.addQuestion("Who scored the winning points?", "Tom");
    sportsStory.addQuestion("What kind of shot won the game?", "three-pointer");
    sports.addReadingMaterial(sportsStory);
}

// Function to handle vocabulary study session
void studyVocabularySession(const Category& category, SubCategory& subCategory, UserProgress& progress) {
    while (true) {
        system("cls");
        cout << "\n=== " << subCategory.name << " ===\n";
        cout << "1. View Words\n";
        cout << "2. Practice Words\n";
        cout << "3. Back to Categories\n";

        string choice;
        getline(cin, choice);

        if (choice == "back" || choice == "3")
            break;

        if (choice == "1") {
            system("cls");
            subCategory.displayWords(progress);
            cout << "\nPress Enter to continue...";
            cin.get();
        }
        else if (choice == "2") {
            system("cls");
            subCategory.studyWords(progress);
            progress.saveToDisk();
            cout << "\nPress Enter to continue...";
            cin.get();
        }
    }
}

// Function to handle reading comprehension session
void readingComprehensionSession(const ReadingComprehension& story) {
    system("cls");
    story.display();
    cout << "\nPress Enter when you're ready for questions...";
    cin.get();

    system("cls");
    story.practice();

    cout << "\nPress Enter to continue...";
    cin.get();
}

void displayMenu() {
    cout << "==========================" << endl;
    cout << "          LEXIMO          " << endl;
    cout << " The English Learning App " << endl;
    cout << "==========================" << endl;
}

int displayMainMenu() {
    const int STUDY_VOCABULARY = 1;
    const int PRACTICE_GRAMMAR = 2;
    const int READ_STORIES = 3;
    const int PRACTICE_TRANSLATION = 4;
    const int VIEW_SKILL_TREE = 5;
    const int VIEW_ACHIEVEMENTS = 6;
    const int VIEW_PROGRESS = 7;
    const int SET_GOALS = 8;
    const int EXIT = 9;
    system("cls");
    cout << "\n=== LEXIMO MAIN MENU ===\n\n";
    cout << "1. Study Vocabulary\n";
    cout << "2. Practice Grammar\n";
    cout << "3. Read Stories\n";
    cout << "4. Practice Translations\n";
    cout << "5. View Skill Tree\n";
    cout << "6. View Achievements\n";
    cout << "7. View Progress\n";
    cout << "8. Set Daily Goals\n";
    cout << "9. Exit\n\n";
    cout << "Enter your choice: ";

    string input;
    getline(cin, input);

    if (input == "back" || input == "exit")
        return EXIT;

    return input[0] - '0';
}

void displayPronunciationGuide() {
    cout << "\n=== Pronunciation Guide ===" << endl;
    cout << "Common Symbols:" << endl;
    cout << "æ - as in 'cat'" << endl;
    cout << "ə - as in 'about'" << endl;
    cout << "ʃ - as in 'ship'" << endl;
    cout << "θ - as in 'think'" << endl;
    cout << "ð - as in 'this'" << endl;
    cout << "\nPress Enter to continue...";
    cin.get();
}

void practiceTranslation(vector<TranslationExercise>& exercises, UserProgress& progress) {
    while (true) {
        system("cls");
        cout << "\n=== Translation Practice ===" << endl;
        cout << "Available exercises:\n";

        for (size_t i = 0; i < exercises.size(); ++i) {
            cout << i + 1 << ". " << exercises[i].word.word
                 << " (" << exercises[i].targetLanguage << ")" << endl;
        }

        cout << "\nEnter exercise number (or 'back'): ";
        string input;
        getline(cin, input);

        if (input == "back")
            break;

        if (isdigit(input[0])) {
            int choice = stoi(input) - 1;
            if (choice >= 0 && choice < exercises.size()) {
                system("cls");
                bool correct = exercises[choice].practice();
                if (correct) {
                    progress.level.addExperience(10);
                    progress.level.totalPoints += 5;
                    progress.checkAchievements();
                }
                cout << "\nPress Enter to continue...";
                cin.get();
            }
        }
    }
}

void studyWord(const Word& word, UserProgress& progress) {
    system("cls");
    word.display();
    cout << "\nPronunciation Guide:" << endl;
    cout << "IPA: " << word.pronunciation << endl;
    cout << "\nPress 1 to mark as learned, or Enter to continue: ";

    string input;
    getline(cin, input);

    if (input == "1") {
        progress.wordProgress[word.word]++;
        progress.wordsLearnedToday++;
        progress.level.addExperience(5);
        progress.level.totalPoints += 2;
        progress.checkAchievements();
        cout << "\nWord marked as learned! +5 XP" << endl;
        cout << "Press Enter to continue...";
        cin.get();
    }
}

// Helper Functions
void initializeSkillTree(vector<SkillNode*>& skillTree) {
    // Basic Skills
    SkillNode* basicVocab = new SkillNode("Basic Vocabulary",
        "Learn essential everyday words", "Beginner");
    SkillNode* basicGrammar = new SkillNode("Basic Grammar",
        "Understand simple sentence structures", "Beginner");

    // Intermediate Skills
    SkillNode* intermediateVocab = new SkillNode("Intermediate Vocabulary",
        "Expand your vocabulary with more complex words", "Intermediate");
    SkillNode* intermediateGrammar = new SkillNode("Intermediate Grammar",
        "Learn compound and complex sentences", "Intermediate");

    // Advanced Skills
    SkillNode* advancedVocab = new SkillNode("Advanced Vocabulary",
        "Master academic and specialized vocabulary", "Advanced");
    SkillNode* advancedGrammar = new SkillNode("Advanced Grammar",
        "Master complex grammatical structures", "Advanced");

    // Set prerequisites
    intermediateVocab->prerequisites.push_back(basicVocab);
    intermediateGrammar->prerequisites.push_back(basicGrammar);
    advancedVocab->prerequisites.push_back(intermediateVocab);
    advancedGrammar->prerequisites.push_back(intermediateGrammar);

    // Add to skill tree
    skillTree = {basicVocab, basicGrammar, intermediateVocab,
                intermediateGrammar, advancedVocab, advancedGrammar};

    // Basic vocab is unlocked by default
    basicVocab->unlocked = true;
    basicGrammar->unlocked = true;
}

void displaySkillTree(const vector<SkillNode*>& skillTree) {
    cout << "\n=== Skill Tree ===" << endl;
    for (const auto& skill : skillTree) {
        cout << (skill->unlocked ? "[X] " : "[ ] ")
             << skill->name << " (" << skill->difficulty << ")" << endl;
        cout << "    " << skill->description << endl;
        if (!skill->prerequisites.empty()) {
            cout << "    Prerequisites: ";
            for (const auto& pre : skill->prerequisites) {
                cout << pre->name << ", ";
            }
            cout << endl;
        }
        cout << endl;
    }
}

void initializeTranslationExercises(vector<TranslationExercise>& exercises) {
    // Spanish translations
    vector<string> options1 = {"perro", "gato", "pájaro", "pez"};
    Word dog("dog", "A faithful pet animal", "/dɔg/");
    dog.addExample("The dog is playing in the garden.");
    exercises.push_back(TranslationExercise(dog, options1, "perro", "Spanish"));

    // French translations
    vector<string> options2 = {"livre", "stylo", "cahier", "crayon"};
    Word book("book", "A written or printed work", "/bʊk/");
    book.addExample("I'm reading an interesting book.");
    exercises.push_back(TranslationExercise(book, options2, "livre", "French"));
}

int main() {
    displayMenu();
    customSleep(2000);
    system("cls");

    string fullName;
    cout << "Please enter your full name: ";
    getline(cin, fullName);

    string firstName;
    size_t spaceIndex = fullName.find(' ');
    if (spaceIndex != string::npos) {
        firstName = fullName.substr(0, spaceIndex);
    } else {
        firstName = fullName;
    }

    customSleep(1000);
    system("cls");

    cout << endl;
    cout << "Hi " << firstName << "," << endl;
    typeMessage("We warmly welcome you to Leximo, your companion in learning English!");

    typeMessage("Press Enter to continue...");
    cin.get();

    vector<Category> vocabulary;
    populateVocabulary(vocabulary);

    vector<GrammarLesson> grammarLessons;
    populateGrammarLessons(grammarLessons);

    UserProgress userProgress(firstName);
    userProgress.loadFromDisk();

    vector<SkillNode*> skillTree;
    initializeSkillTree(skillTree);

    vector<TranslationExercise> translationExercises;
    initializeTranslationExercises(translationExercises);

    chrono::system_clock::time_point lastLoginTime;



    while (true) {
        int choice = displayMainMenu();

        if (choice == 1) {
            while (true) {
                system("cls");
                cout << "\nChoose a category:\n\n";
                for (size_t i = 0; i < vocabulary.size(); ++i) {
                    cout << i + 1 << ". " << vocabulary[i].name << endl;
                }
                cout << "\nEnter choice (or 'back'): ";

                string input;
                getline(cin, input);

                if (input == "back")
                    break;

                if (isdigit(input[0])) {
                    int catChoice = stoi(input) - 1;
                    if (catChoice >= 0 && catChoice < vocabulary.size()) {
                        while (true) {
                            system("cls");
                            cout << "\nChoose a subcategory in " << vocabulary[catChoice].name << ":\n\n";
                            for (size_t i = 0; i < vocabulary[catChoice].subcategories.size(); ++i) {
                                cout << i + 1 << ". " << vocabulary[catChoice].subcategories[i].name << endl;
                            }
                            cout << "\nEnter choice (or 'back'): ";

                            string subInput;
                            getline(cin, subInput);

                            if (subInput == "back")
                                break;

                            if (isdigit(subInput[0])) {
                                int subChoice = stoi(subInput) - 1;
                                if (subChoice >= 0 && subChoice < vocabulary[catChoice].subcategories.size()) {
                                    studyVocabularySession(vocabulary[catChoice],
                                                         vocabulary[catChoice].subcategories[subChoice],
                                                         userProgress);
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (choice == 2) {
            while (true) {
                system("cls");
                cout << "\nGrammar Lessons:\n\n";
                for (size_t i = 0; i < grammarLessons.size(); ++i) {
                    cout << i + 1 << ". " << grammarLessons[i].title << endl;
                }
                cout << "\nEnter choice (or 'back'): ";

                string input;
                getline(cin, input);

                if (input == "back")
                    break;

                if (isdigit(input[0])) {
                    int lessonChoice = stoi(input) - 1;
                    if (lessonChoice >= 0 && lessonChoice < grammarLessons.size()) {
                        system("cls");
                        grammarLessons[lessonChoice].display();
                        cout << "\nPress Enter to start practice...";
                        cin.get();
                        grammarLessons[lessonChoice].practice();
                        userProgress.completedLessons.push_back(grammarLessons[lessonChoice].title);
                        cout << "\nPress Enter to continue...";
                        cin.get();
                    }
                }
            }
        }
        else if (choice == 3) {
            while (true) {
                system("cls");
                cout << "\nChoose a category for reading:\n\n";
                for (size_t i = 0; i < vocabulary.size(); ++i) {
                    cout << i + 1 << ". " << vocabulary[i].name << endl;
                }
                cout << "\nEnter choice (or 'back'): ";

                string input;
                getline(cin, input);

                if (input == "back")
                    break;

                if (isdigit(input[0])) {
                    int catChoice = stoi(input) - 1;
                    if (catChoice >= 0 && catChoice < vocabulary.size()) {
                        while (true) {
                            system("cls");
                            vocabulary[catChoice].displayReadingMaterials();
                            cout << "\nEnter choice (or 'back'): ";

                            string storyInput;
                            getline(cin, storyInput);

                            if (storyInput == "back")
                                break;

                            if (isdigit(storyInput[0])) {
                                int storyChoice = stoi(storyInput) - 1;
                                if (storyChoice >= 0 && storyChoice < vocabulary[catChoice].readingMaterials.size()) {
                                    readingComprehensionSession(vocabulary[catChoice].readingMaterials[storyChoice]);
                                }
                            }
                        }
                    }
                }
            }
        }

        else if (choice == 4) {
            practiceTranslation(translationExercises, userProgress);
        }
        else if (choice == 5) {
            system("cls");
            displaySkillTree(skillTree);
            cout << "\nPress Enter to continue...";
            cin.get();
        }
        else if (choice == 6) {
            system("cls");
            cout << "\n=== Achievements ===" << endl;
            for (const auto& achievement : userProgress.achievements) {
                cout << (achievement.unlocked ? "[X] " : "[ ] ")
                     << achievement.name << " - " << achievement.description
                     << " (" << achievement.pointsReward << " points)" << endl;
            }
            cout << "\nPress Enter to continue...";
            cin.get();
        }
        else if (choice == 7) {
            system("cls");
            userProgress.displayProgress();
            cout << "\nPress Enter to continue...";
            cin.get();
        }
        else if (choice == 8) {
            system("cls");
            cout << "Current daily goal: " << userProgress.dailyGoal << " words\n";
            cout << "Enter new daily goal: ";
            string input;
            getline(cin, input);
            if (isdigit(input[0])) {
                userProgress.dailyGoal = stoi(input);
                cout << "\nDaily goal updated successfully!\n";
            }
            cout << "\nPress Enter to continue...";
            cin.get();
        }
        else if (choice == 9) {
            userProgress.saveToDisk();
            typeMessage("Thank you for using Leximo! Have a great day ahead, " + firstName + "!");
            customSleep(1000);
            return 0;
        }
        else {
            cout << "Invalid choice. Please try again.\n";
            customSleep(1000);
        }


        // Check and update streak
        auto now = chrono::system_clock::now();
        auto duration = chrono::duration_cast<chrono::hours>(
            now - userProgress.lastLoginTime).count();

        if (duration <= 24) {
            userProgress.level.incrementStreak();
        } else {
            userProgress.level.resetStreak();
        }

        userProgress.lastLoginTime = now;
        userProgress.checkAchievements();
        userProgress.saveToDisk();
    }
     for (auto* skill : skillTree) {
        delete skill;
    }

    return 0;
}

