#include <iostream>
#include <SFML/Audio.hpp>
#include <filesystem>
#include <map>
#include <thread>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <conio.h> // For _getch()
#include <chrono>  // For timing
#include <fstream>
#include <windows.h>
#include <queue>
#include <stack>
#include <unordered_map>

using namespace std;

struct Message {
    string text;
    string audioFile;
};

struct User {
    string username;
    string password;
    int proficiencyLevel;
    int dailyGoal;
    string source;
    string learningReason;
};

struct Question {
    string text;
    vector<string> options;
    int correctAnswer;
    string audioFile;
    bool isPronunciation;
    int difficulty;
};

struct AudioNode {
    string identifier;
    sf::SoundBuffer buffer;
    AudioNode* next;
    AudioNode(string id) : identifier(id), next(nullptr) {}
};

// Binary Tree Node for questions
struct QuestionTreeNode {
    Question data;
    QuestionTreeNode* left;
    QuestionTreeNode* right;
    QuestionTreeNode(Question q) : data(q), left(nullptr), right(nullptr) {}
};

// Hash Table Node for audio files
struct HashNode {
    string key;
    sf::SoundBuffer value;
    HashNode* next;
    HashNode(string k, sf::SoundBuffer v) : key(k), value(v), next(nullptr) {}
};

// Custom Hash Table implementation for audio files
class AudioHashTable {
private:
    static const int TABLE_SIZE = 101;  // Prime number for better distribution
    HashNode* table[TABLE_SIZE];
    
    int hashFunction(string key) {
        int hash = 0;
        for (char c : key) {
            hash = (hash * 31 + c) % TABLE_SIZE;
        }
        return hash;
    }

public:
    AudioHashTable() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            table[i] = nullptr;
        }
    }
    
    void insert(string key, sf::SoundBuffer value) {
        int index = hashFunction(key);
        HashNode* newNode = new HashNode(key, value);
        
        if (table[index] == nullptr) {
            table[index] = newNode;
        } else {
            newNode->next = table[index];
            table[index] = newNode;
        }
    }
    
    sf::SoundBuffer* get(string key) {
        int index = hashFunction(key);
        HashNode* current = table[index];
        
        while (current != nullptr) {
            if (current->key == key) {
                return &(current->value);
            }
            current = current->next;
        }
        return nullptr;
    }
};

// Utility functions
void gotoRowCol(int row, int col) {
    COORD coord;
    coord.X = col;
    coord.Y = row;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void clearScreen() {
    system("cls");
}

void displayLogo() {
    clearScreen();
    int centerRow = 10;
    int centerCol = 35;

    gotoRowCol(centerRow, centerCol);
    cout << "L E X I M O\n";
    gotoRowCol(centerRow + 2, centerCol - 5);
    cout << "Your English Learning Companion\n";
    gotoRowCol(centerRow + 4, centerCol - 10);
}

class AudioManager {
private:
    AudioHashTable audioFiles;
    sf::Sound sound;
    string audioPath;

public:
    AudioManager(const string& basePath) : audioPath(basePath) {}

    bool loadAudio(const string& identifier, const string& filename) {
        sf::SoundBuffer buffer;
        string fullPath = audioPath + "/" + filename;
        
        if (!buffer.loadFromFile(fullPath)) {
            cerr << "Warning: Could not load audio file '" << fullPath << "' - Exercise will continue without audio" << endl;
            return false;
        }
        
        audioFiles.insert(identifier, buffer);
        return true;
    }

    void playAudio(const string& identifier) {
        sf::SoundBuffer* buffer = audioFiles.get(identifier);
        if (buffer != nullptr) {
            sound.setBuffer(*buffer);
            sound.play();
            
            while (sound.getStatus() == sf::Sound::Playing) {
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        } else {
            cout << "\n[Audio playback not available for this question]\n";
            Sleep(1000);
        }
    }
};


class UserManager {
private:
    const string USER_FILE = "users.txt";

public:
    bool isValidUsername(const string& username) {
        if (username.empty()) return false;
        for (char c : username) {
            if (isspace(c) || (!isalnum(c) && c != '_' && c != '-')) 
                return false;
        }
        return true;
    }

    bool isValidPassword(const string& password) {
        if (password.length() < 8) return false;
        int wordCount = 1;
        for (char c : password) {
            if (isspace(c)) wordCount++;
        }
        return (password.length() >= 8) || (wordCount >= 8);
    }

    bool saveUser(const User& user) {
        ofstream file(USER_FILE, ios::app);
        if (!file) return false;
        file << user.username << "," << user.password << "\n";
        return true;
    }

    bool usernameExists(const string& username) {
        ifstream file(USER_FILE);
        string line;
        while (getline(file, line)) {
            size_t pos = line.find(",");
            if (pos != string::npos && line.substr(0, pos) == username) 
                return true;
        }
        return false;
    }

    bool verifyLogin(const string& username, const string& password) {
        ifstream file(USER_FILE);
        string line;
        while (getline(file, line)) {
            size_t pos = line.find(",");
            if (pos != string::npos) {
                if (line.substr(0, pos) == username && 
                    line.substr(pos + 1) == password) {
                    return true;
                }
            }
        }
        return false;
    }
};

// Question Bank using Binary Search Tree and Linked Lists
class QuestionBank {
private:
    struct QuestionList {
        Question data;
        QuestionList* next;
        QuestionList(Question q) : data(q), next(nullptr) {}
    };

    QuestionTreeNode* root;
    QuestionList* difficultyLists[5];  // Array of linked lists for each difficulty level
    
    void insertIntoTree(QuestionTreeNode*& node, Question q) {
        if (node == nullptr) {
            node = new QuestionTreeNode(q);
            return;
        }
        
        if (q.difficulty < node->data.difficulty) {
            insertIntoTree(node->left, q);
        } else {
            insertIntoTree(node->right, q);
        }
    }
    
    void insertIntoList(int difficulty, Question q) {
        QuestionList* newNode = new QuestionList(q);
        newNode->next = difficultyLists[difficulty - 1];
        difficultyLists[difficulty - 1] = newNode;
    }

public:
    QuestionBank() : root(nullptr) {
        for (int i = 0; i < 5; i++) {
            difficultyLists[i] = nullptr;
        }
        initializeQuestions();
    }
    
    void addQuestion(Question q) {
        insertIntoTree(root, q);
        insertIntoList(q.difficulty, q);
    }
    
    void initializeQuestions() {
        // Add sample questions for each difficulty level
        // Beginner level (1) questions
        vector<Question> questions = {
            {"What is the correct article? _ apple", {"a", "an", "the"}, 2, "apple_article.wav", false, 1},
            {"Listen and repeat: Hello", {}, 1, "hello.wav", true, 1},
            {"Complete the sentence: Good __ ! (morning greeting)", {"night", "morning", "evening"}, 2, "morning_greet.wav", false, 1},
            
            // Intermediate level (3) questions
            {"Choose the correct form: If it _ tomorrow, I will stay home", {"rains", "rain", "raining"}, 1, "if_clause.wav", false, 3},
            {"Listen and repeat: Opportunity", {}, 1, "opportunity.wav", true, 3},
            {"Select the correct word: She _ to the store yesterday", {"gone", "went", "going"}, 2, "past_tense.wav", false, 3},
            
            // Advanced level (5) questions
            {"Choose: Despite _ early, we missed the train", {"leaving", "left", "leave"}, 1, "despite_clause.wav", false, 5},
            {"Listen and pronounce: Particularly difficult", {}, 1, "difficult_phrase.wav", true, 5},
            {"Complete: The report _ by tomorrow morning", {"will finish", "will be finished", "finishing"}, 2, "passive_future.wav", false, 5}
        };
        
        for (const Question& q : questions) {
            addQuestion(q);
        }
    }
    
    vector<Question> getQuestionsByProficiency(int proficiency, int count) {
        vector<Question> result;
        
        // Convert proficiency to difficulty level
        int difficulty = (proficiency <= 2) ? 1 : (proficiency <= 4) ? 3 : 5;
        
        // Get questions from the appropriate difficulty list
        QuestionList* current = difficultyLists[difficulty - 1];
        while (current != nullptr && result.size() < count) {
            result.push_back(current->data);
            current = current->next;
        }
        
        // If we don't have enough questions, add default ones
        while (result.size() < count) {
            Question defaultQ = {
                "What is the meaning of 'Hello'?",
                {"A greeting", "A farewell", "A question"},
                1,
                "hello.wav",
                false,
                difficulty
            };
            result.push_back(defaultQ);
        }
        
        return result;
    }
};

// Stack for wrong answers
class WrongAnswerStack {
private:
    struct StackNode {
        Question data;
        StackNode* next;
        StackNode(Question q) : data(q), next(nullptr) {}
    };
    
    StackNode* top;
    
public:
    WrongAnswerStack() : top(nullptr) {}
    
    void push(Question q) {
        StackNode* newNode = new StackNode(q);
        newNode->next = top;
        top = newNode;
    }
    
    bool pop(Question& q) {
        if (top == nullptr) return false;
        
        q = top->data;
        StackNode* temp = top;
        top = top->next;
        delete temp;
        return true;
    }
    
    bool isEmpty() {
        return top == nullptr;
    }
};

// Queue for review questions
class ReviewQueue {
private:
    struct QueueNode {
        Question data;
        QueueNode* next;
        QueueNode(Question q) : data(q), next(nullptr) {}
    };
    
    QueueNode *front, *rear;
    
public:
    ReviewQueue() : front(nullptr), rear(nullptr) {}
    
    void enqueue(Question q) {
        QueueNode* newNode = new QueueNode(q);
        
        if (rear == nullptr) {
            front = rear = newNode;
            return;
        }
        
        rear->next = newNode;
        rear = newNode;
    }
    
    bool dequeue(Question& q) {
        if (front == nullptr) return false;
        
        q = front->data;
        QueueNode* temp = front;
        front = front->next;
        
        if (front == nullptr) {
            rear = nullptr;
        }
        
        delete temp;
        return true;
    }
    
    bool isEmpty() {
        return front == nullptr;
    }
};

class ProgressTracker {
private:
    stack<Question> wrongAnswers;
    queue<Question> reviewQueue;
    int streakCount;
    
public:
    ProgressTracker() : streakCount(0) {}
    
    void addWrongAnswer(const Question& q) {
        wrongAnswers.push(q);
    }
    
    void prepareReview() {
        while (!wrongAnswers.empty()) {
            reviewQueue.push(wrongAnswers.top());
            wrongAnswers.pop();
        }
    }
    
    bool hasQuestionsForReview() {
        return !reviewQueue.empty();
    }
    
    Question getNextReviewQuestion() {
        Question q = reviewQueue.front();
        reviewQueue.pop();
        return q;
    }
    
    void incrementStreak() {
        streakCount++;
    }
    
    int getStreak() {
        return streakCount;
    }
    
    void displayProgressBar(int row, int col, int progress) {
        gotoRowCol(row, col);
        cout << "Progress: [";
        for (int i = 0; i < progress; i++) {
            cout << "=";
        }
        for (int i = progress; i < 10; i++) {
            cout << " ";
        }
        cout << "]";
    }
};


class LeximoApp {
private:
    AudioManager audioManager;
    UserManager userManager;
    QuestionBank questionBank;
    ProgressTracker progressTracker;
    vector<Message> messages;
    map<int, Message> proficiencyResponses;
    User currentUser;
    
   void displayMessage(const string& text, bool playAudio = true, string audioFile = "") {
        clearScreen();
        displayLogo();
        cout << "\n╔════════════════════════════════════════════╗\n";
        cout << "║ " << text;
        int padding = 38 - text.length();
        if (padding > 0) {
            cout << string(padding, ' ');
        }
        cout << " ║\n";
        cout << "╚════════════════════════════════════════════╝\n\n";
        
        if (playAudio && !audioFile.empty()) {
            audioManager.playAudio(audioFile);
        }
    }

    int getValidInput(int min, int max) {
        int choice;
        while (true) {
            cout << "Enter your choice (" << min << "-" << max << "): ";
            if (cin >> choice && choice >= min && choice <= max) {
                cin.ignore();
                return choice;
            }
            cout << "Invalid input. Please try again.\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }

    User handleSignup() {
        User user;
        string username, password;
        bool validUsername = false;

        do {
            clearScreen();
            displayLogo();
            gotoRowCol(15, 25);
            cout << "Create Username: ";
            getline(cin, username);

            if (userManager.usernameExists(username)) {
                gotoRowCol(17, 25);
                cout << "Username already taken!\n";
                Sleep(1500);
                continue;
            }

            if (!userManager.isValidUsername(username)) {
                gotoRowCol(17, 25);
                cout << "Username can only contain letters, numbers, - or _\n";
                Sleep(1500);
                continue;
            }

            validUsername = true;
        } while (!validUsername);

        bool validPassword = false;
        do {
            clearScreen();
            displayLogo();
            gotoRowCol(15, 25);
            cout << "Username: " << username << "\n";
            gotoRowCol(16, 25);
            cout << "Create Password: ";
            getline(cin, password);

            if (!userManager.isValidPassword(password)) {
                gotoRowCol(17, 25);
                cout << "Password must be 8 or more characters/words combined!\n";
                Sleep(2000);
                continue;
            }

            validPassword = true;
        } while (!validPassword);

        user.username = username;
        user.password = password;
        userManager.saveUser(user);
        return user;
    }

    
    
    void runPracticeExercise() {
        clearScreen();
        displayLogo();
        
        cout << "\nHere's your first day streak! Let's begin the practice exercise.\n\n";
        Sleep(2000);
        
        int baseQuestions;
        if (currentUser.dailyGoal <= 5) baseQuestions = 7;
        else if (currentUser.dailyGoal <= 10) baseQuestions = 15;
        else if (currentUser.dailyGoal <= 15) baseQuestions = 40; 
        
        vector<Question> questions = questionBank.getQuestionsByProficiency(
            currentUser.proficiencyLevel, 
            baseQuestions
        );
        
        int progress = 0;
        for (const Question& q : questions) {
            displayQuestion(q);
            if (!handleAnswer(q)) {
                progressTracker.addWrongAnswer(q);
            }
            progress++;
            progressTracker.displayProgressBar(20, 30, (progress * 10) / questions.size());
        }
        
        // Handle review of wrong answers
        if (progressTracker.hasQuestionsForReview()) {
            cout << "\nLet's review the questions you missed:\n";
            Sleep(1500);
            
            progressTracker.prepareReview();
            while (progressTracker.hasQuestionsForReview()) {
                Question q = progressTracker.getNextReviewQuestion();
                displayQuestion(q);
                handleAnswer(q);
            }
        }
        
        progressTracker.incrementStreak();
        cout << "\nCongratulations! You've completed your first practice session!\n";
        cout << "Current streak: " << progressTracker.getStreak() << " day(s)\n";
        Sleep(3000);
    }
    
    void displayQuestion(const Question& q) {
        clearScreen();
        displayLogo();
        
        cout << "\nQuestion: " << q.text << "\n\n";
        
        if (q.isPronunciation) {
            audioManager.playAudio(q.audioFile);
            cout << "Listen to the audio and repeat.\n";
        } else {
            for (size_t i = 0; i < q.options.size(); i++) {
                cout << i + 1 << ". " << q.options[i] << "\n";
            }
        }
    }
    
    bool handleAnswer(const Question& q) {
        if (q.isPronunciation) {
            cout << "\nPress any key when you're done practicing...\n";
           // cin.ignore();
            cin.get();
            return true; // Pronunciation practice is always marked as correct
        } else {
            cout << "\nEnter your answer (1-" << q.options.size() << "): ";
            int answer;
            cin >> answer;
            cin.ignore();
            
            return answer == q.correctAnswer;
        }
    }

public:
    
    User handleLogin() {
        User user;
        string username, password;
        bool validUsername = false;

        while (!validUsername) {
            clearScreen();
            displayLogo();
            gotoRowCol(15, 25);
            cout << "Username: ";
            cin >> username;

            if (!userManager.usernameExists(username)) {
                gotoRowCol(17, 25);
                cout << "Username does not exist!\n";
                Sleep(1500);
                continue;
            }
            validUsername = true;
        }

        bool loginSuccess = false;
        do {
            clearScreen();
            displayLogo();
            gotoRowCol(15, 25);
            cout << "Username: " << username << "\n";
            gotoRowCol(16, 25);
            cout << "Password: ";
            cin >> password;

            if (userManager.verifyLogin(username, password)) {
                loginSuccess = true;
                user.username = username;
                user.password = password;
            } else {
                gotoRowCol(18, 25);
                cout << "Incorrect password!\n";
                Sleep(2000);
            }
        } while (!loginSuccess);

        return user;
    }
    LeximoApp() : audioManager(filesystem::current_path().string() + "/Audiofiles") {
        messages = {
            {"Welcome to Leximo!", "first"},  // Welcome message at start
            {"Hi there! I am Leximo.", "11 (17)"},
            {"Just 4 quick questions before we start your first lesson!", "quickquestions"},
            {"How did you hear about Leximo?", "11 (13)"},
            {"How much English do you know?", "11 (11)"},
            {"Why are you learning English?", "whyenglish"},
            {"Great reason to learn!", "greatreason"},
            {"Let's setup your daily learning goal.", "dailygoal"}
        };

        proficiencyResponses = {
            {1, {"Okay! We will start fresh.", "11 (20)"}},
            {2, {"Okay! We will build on what you know.", "buildon"}},
            {3, {"Okay! We will build on what you know.", "buildon"}},
            {4, {"Wow! That's great.", "wowgreat"}},
            {5, {"Wow! That's great.", "wowgreat"}}
        };

        // Load audio files
        for (const auto& msg : messages) {
            audioManager.loadAudio(msg.audioFile, msg.audioFile + ".wav");
        }
        for (const auto& resp : proficiencyResponses) {
            audioManager.loadAudio(resp.second.audioFile, resp.second.audioFile + ".wav");
        }
    }

    void runInitialQuestionnaire() {
        // Introduction
        displayMessage(messages[1].text, true, messages[1].audioFile);  // Hi there! I am Leximo
        Sleep(2000);
        
        displayMessage(messages[2].text, true, messages[2].audioFile);  // Just 5 quick questions
        Sleep(2000);

        // Source question
        displayMessage(messages[3].text, true, messages[3].audioFile);  // How did you hear about Leximo?
        cout << "1. Social Media\n";
        cout << "2. Article\n\n";
        int source = getValidInput(1, 2);
        currentUser.source = (source == 1) ? "Social Media" : "Article";

        // English proficiency
        displayMessage(messages[4].text, true, messages[4].audioFile);
        cout << "1. I am new to English\n";
        cout << "2. I know some common words\n";
        cout << "3. I can have basic conversations\n";
        cout << "4. I can talk about various topics\n";
        cout << "5. I can discuss most topics in detail\n\n";
        
        int proficiency = getValidInput(1, 5);
        currentUser.proficiencyLevel = proficiency;
        displayMessage(proficiencyResponses[proficiency].text, true, proficiencyResponses[proficiency].audioFile);
        Sleep(2000);

        // Learning goals
        displayMessage(messages[5].text, true, messages[5].audioFile);  // Why are you learning English?
        cout << "1. Support my education\n";
        cout << "2. Connect with people\n";
        cout << "3. Boost my career\n\n";
        int goal = getValidInput(1, 3);
        string goals[] = {"Education", "Social", "Career"};
        currentUser.learningReason = goals[goal - 1];

        // Show response to learning goals
        displayMessage(messages[6].text, true, messages[6].audioFile);  // Great reasons to learn!
        Sleep(2000);

        // Daily goal setting
        vector<int> dailyGoals = {5, 10, 15, 30};
        displayMessage(messages[7].text, true, messages[7].audioFile);  // Great reasons to learn!
        Sleep(2000);
        for (size_t i = 0; i < dailyGoals.size(); i++) {
            cout << i + 1 << ". " << dailyGoals[i] << " minutes/day\n";
        }
        cout << "\n";
        
        int dailyChoice = getValidInput(1, dailyGoals.size());
        cout << "Great! You'll learn approximately " << dailyChoice * 7 << " words per week!";
        cout << "\n";
        currentUser.dailyGoal = dailyGoals[dailyChoice - 1];
        cout << "\n";
        

    }

    
    void run() {
        displayLogo();
        gotoRowCol(15, 30);
        cout << "1. Get Started\n";
        gotoRowCol(16, 30);
        cout << "2. I Already Have an Account\n";
        gotoRowCol(18, 30);
        cout << "Enter your choice: ";

        int choice = getValidInput(1, 2);

        if (choice == 1) {
            displayMessage(messages[0].text, true, messages[0].audioFile);
            Sleep(2000);
            
            currentUser = handleSignup();
            runInitialQuestionnaire();
            runPracticeExercise(); // Add this line to start practice after questionnaire
        } else {
            currentUser = handleLogin();
        }

        clearScreen();
        displayLogo();
        gotoRowCol(15, 25);
        cout << "Welcome to LEXIMO, " << currentUser.username << "!\n";
        Sleep(2000);
    }
};
// Forward declarations
class Word;
class Category;
class Ques;
class Story;

// Structure for a word and its properties
class Word {
public:
    string word;
    string meaning;
    bool isLearned;

    Word(string w, string m) : word(w), meaning(m), isLearned(false) {}
};

// Structure for a category of words
class Category {
public:
    string name;
    vector<Word> words;
    bool isUnlocked;

    Category(string n) : name(n), isUnlocked(true) {}

    void addWord(string word, string meaning) {
        words.push_back(Word(word, meaning));
    }

    void displayWords() {
        cout << "\n=== " << name << " ===\n";
        for (int i = 0; i < words.size(); i++) {
            cout << i + 1 << ". " << words[i].word << endl;
        }
    }
};

// Structure for a story with comprehension questions
class Story {
public:
    string title;
    string content;
    vector<Ques*> questions;

    Story(string t, string c) : title(t), content(c) {}

    void display() const {
        cout << "\n=== " << title << " ===\n\n";
        cout << content << endl;
    }
};

// Structure for multiple choice questions
class Ques {
public:
    string question;
    vector<string> options;
    int correctAnswer;
    bool isAnswered;
    bool isCorrect;

    Ques(string q, vector<string> opts, int correct)
        : question(q), options(opts), correctAnswer(correct),
          isAnswered(false), isCorrect(false) {}

    void display(int num) {
        cout << "\nQuestion " << num << ": " << question << endl;
        for (int i = 0; i < options.size(); i++) {
            cout << char('A' + i) << ") " << options[i] << endl;
        }
    }

    bool checkAnswer(char answer) {
        isAnswered = true;
        int userAnswer = toupper(answer) - 'A';
        isCorrect = (userAnswer == correctAnswer);
        return isCorrect;
    }

    void showCorrectAnswer() {
        cout << "The correct answer is: " << char('A' + correctAnswer)
             << ") " << options[correctAnswer] << endl;
    }
};

// Main application class
class LanguageLearningApp {
private:
    vector<Category> categories;
    vector<Story> stories;
    vector<Ques*> allQuestions;
    queue<Ques*> mistakeQueue;
    string userName;
    int score;
    int totalWords;
    int wordsLearned;

    void initializeCategories() {
        // Animals Category
        system("cls");
        Category animals("Animals");
animals.addWord("Hippopotamus", "Large semiaquatic mammal found in Africa (pronounced: hip-uh-pot-uh-muhs)");
animals.addWord("Rhinoceros", "Large, herbivorous mammal with a horn (pronounced: rye-noss-er-us)");
animals.addWord("Cheetah", "Fastest land animal (pronounced: chee-tuh)");
animals.addWord("Giraffe", "Tallest animal (pronounced: juh-raf)");
animals.addWord("Penguin", "Flightless bird from Antarctica (pronounced: peng-gwin)");
animals.addWord("Zebra", "Striped herbivorous animal from Africa (pronounced: zee-bruh or zeh-bruh)");
animals.addWord("Octopus", "Marine creature with eight tentacles (pronounced: ok-tuh-pus)");
animals.addWord("Platypus", "Egg-laying mammal from Australia (pronounced: plat-uh-pus)");
categories.push_back(animals);

        // Professions Category
Category professions("Professions");

// Medical professions
professions.addWord("Doctor", "Medical professional who treats patients");
professions.addWord("Nurse", "Healthcare professional who cares for patients");
professions.addWord("Surgeon", "Doctor who performs operations");
professions.addWord("Pediatrician", "Doctor specializing in child healthcare");
professions.addWord("Dentist", "Doctor specializing in oral health");
professions.addWord("Pharmacist", "Professional who prepares and dispenses medicines");

// Tech professions
professions.addWord("Programmer", "Person who writes computer code");
professions.addWord("Engineer", "Professional who designs and builds things");
professions.addWord("Analyst", "Person who analyzes data and systems");
professions.addWord("Designer", "Person who creates visual and user experiences");
professions.addWord("Developer", "Person who builds software applications");

categories.push_back(professions);

Category fruits("Fruits");
fruits.addWord("Apple", "Round fruit that can be red or green");
fruits.addWord("Banana", "Long yellow fruit");
fruits.addWord("Orange", "Round citrus fruit");
fruits.addWord("Grape", "Small, sweet fruit growing in clusters");
fruits.addWord("Mango", "Sweet tropical fruit");

Category vegetables("Vegetables");
vegetables.addWord("Carrot", "Orange root vegetable");
vegetables.addWord("Potato", "Starchy root vegetable");
vegetables.addWord("Tomato", "Red fruit used as a vegetable");
vegetables.addWord("Lettuce", "Leafy green vegetable");
vegetables.addWord("Cucumber", "Long green vegetable");

categories.push_back(fruits);
categories.push_back(vegetables);

Category technology("Technology");

// Devices category
technology.addWord("Smartphone", "Mobile phone with advanced computing capability");
technology.addWord("Laptop", "Portable computer");
technology.addWord("Tablet", "Portable touchscreen device");
technology.addWord("Smartwatch", "Wearable computing device");

// Software category
technology.addWord("App", "Application program for devices");
technology.addWord("Browser", "Program for accessing the internet");
technology.addWord("Operating System", "Basic software managing computer hardware");
technology.addWord("Antivirus", "Program protecting against malicious software");

categories.push_back(technology);

Category travel("Travel");

// Transportation category
travel.addWord("Airplane", "Aircraft for air travel");
travel.addWord("Train", "Rail transport vehicle");
travel.addWord("Bus", "Large road vehicle for passengers");
travel.addWord("Taxi", "Car service with driver");

// Accommodation category
travel.addWord("Hotel", "Establishment providing paid lodging");
travel.addWord("Hostel", "Budget-friendly shared accommodation");
travel.addWord("Resort", "Place for relaxation and recreation");
travel.addWord("Motel", "Roadside hotel for motorists");

categories.push_back(travel);

Category sports("Sports");

// Team sports category
sports.addWord("Football", "Sport played with a ball between two teams");
sports.addWord("Basketball", "Sport played with a ball and hoops");
sports.addWord("Volleyball", "Sport played over a net");

// Individual sports category
sports.addWord("Swimming", "Sport of moving through water");
sports.addWord("Tennis", "Sport played with rackets");
sports.addWord("Golf", "Sport played with clubs and balls");

categories.push_back(sports);

        totalWords = animals.words.size() + professions.words.size() + fruits.words.size() + vegetables.words.size() + technology.words.size() + travel.words.size() +  sports.words.size();
    }

    void initializeStories() {

// Story 1
Story story1("The Maverick Woman",
    "Sophia had always been a maverick in her career. While many followed the traditional path, she evolved her own unique way of doing things.\n"
    "As a rebel kid, she had always questioned authority, and that spirit stayed with her into adulthood.\n"
    "Her colleagues found her nonchalant, never too worried about the opinions of others, but deep down, she knew she was vulnerable to the pressures of succeeding in a male-dominated industry.\n"
    "Despite the challenges, Sophia remained determined to push forward, carving her own path with confidence.\n");
stories.push_back(story1);

// Story 2
Story story2("The Courageous Decision",
    "Emily had always been the quiet one in the group, known for her gentle nature and thoughtful demeanor.\n"
    "Her friends often admired how she could stay calm, even in stressful situations.\n"
    "But beneath her composed exterior, Emily faced moments of uncertainty.\n"
    "When her company faced a major crisis, most of her colleagues panicked, but Emily made a bold decision to take charge.\n"
    "Though she felt uncertain at times, she found the strength to lead the team through the challenge.\n"
    "By the end of the month, her leadership had turned the situation around, earning her the respect of even the toughest critics.\n"
    "Emily learned that courage isn't about being fearless, but about making the right choices when it counts the most.\n");
stories.push_back(story2);

// Story 3
Story story3("The Hidden Kingdom",
    "Lena had always felt like she was meant for something greater.\n"
    "One evening, while exploring the forest near her home, she stumbled upon a glowing stone.\n"
    "When she touched it, the ground beneath her feet began to shake, and a hidden path appeared.\n"
    "Intrigued and nervous, she followed the path, which led her to an ancient, hidden kingdom.\n"
    "The kingdom was filled with creatures she’d only heard of in legends, like talking wolves and dragons that soared across the sky.\n"
    "The queen of the kingdom, a wise and kind woman, greeted Lena with open arms, telling her that she was destined to help them save their world from an impending darkness.\n"
    "As Lena began to uncover her magical powers, she realized her life was about to change in ways she never imagined.\n");
stories.push_back(story3);

    }

 void initializeQuestions() {
    // Questions for Story 1 ("The Maverick Woman")
    Ques* q1 = new Ques(
        "In 'The Maverick Woman,' what trait made Sophia stand out in her career?",
        {"She always followed traditional paths.",
         "She had a unique way of doing things.",
         "She avoided taking any risks.",
         "She only focused on the opinions of others."},
        1
    );
    allQuestions.push_back(q1);

    Ques* q2 = new Ques(
        "In 'The Maverick Woman,' how did Sophia feel about succeeding in a male-dominated industry?",
        {"Confident and unbothered.",
         "Vulnerable to the pressures of success.",
         "Indifferent about it.",
         "Completely uninterested in the industry."},
        1
    );
    allQuestions.push_back(q2);

    Ques* q3 = new Ques(
        "What does the word 'maverick' mean in the context of 'The Maverick Woman'?",
        {"A person who follows the crowd.",
         "A person who takes an independent stand.",
         "A person who dislikes change.",
         "A person who always agrees with others."},
        1
    );
    allQuestions.push_back(q3);

    Ques* q4 = new Ques(
        "What does 'vulnerable' mean in the context of 'The Maverick Woman'?",
        {"Strong and unbreakable.",
         "Susceptible to emotional or physical harm.",
         "Indifferent to challenges.",
         "Unaffected by external pressures."},
        1
    );
    allQuestions.push_back(q4);

    // Questions for Story 2 ("The Courageous Decision")
    Ques* q5 = new Ques(
        "What did Emily do during the crisis in her company in 'The Courageous Decision'?",
        {"She panicked along with her colleagues.",
         "She ignored the crisis and went on a vacation.",
         "She made a bold decision to take charge.",
         "She chose not to participate in the solution."},
        2
    );
    allQuestions.push_back(q5);

    Ques* q6 = new Ques(
        "In 'The Courageous Decision,' what quality did Emily demonstrate during the crisis?",
        {"Leadership and strength.",
         "Indifference to the situation.",
         "Fear and panic.",
         "Laziness and lack of initiative."},
        0
    );
    allQuestions.push_back(q6);

    Ques* q7 = new Ques(
        "In 'The Courageous Decision,' what does the word 'composed' mean?",
        {"Nervous and anxious.",
         "Calm and self-controlled.",
         "Angry and upset.",
         "Excited and energetic."},
        0
    );
    allQuestions.push_back(q7);

    // Questions for Story 3 ("The Hidden Kingdom")
    Ques* q8 = new Ques(
        "In 'The Hidden Kingdom,' what did Lena discover while exploring the forest?",
        {"A hidden treasure chest.",
         "A glowing stone.",
         "A magical creature.",
         "A secret portal to another world."},
        1
    );
    allQuestions.push_back(q8);

    Ques* q9 = new Ques(
        "In 'The Hidden Kingdom,' what creatures did Lena encounter in the hidden kingdom?",
        {"Unicorns and mermaids.",
         "Talking wolves and dragons.",
         "Fairies and trolls.",
         "Ghosts and vampires."},
        1
    );
    allQuestions.push_back(q9);

    Ques* q10 = new Ques(
        "What does 'intrigued' mean in the context of 'The Hidden Kingdom'?",
        {"Feeling uninterested.",
         "Feeling confused.",
         "Aroused curiosity or interest.",
         "Feeling scared."},
        2
    );
    allQuestions.push_back(q10);
}

public:
    LanguageLearningApp() : score(0), totalWords(0), wordsLearned(0) {

        initializeCategories();
        initializeStories();
        initializeQuestions();
    }

    ~LanguageLearningApp() {
        for (Ques* q : allQuestions) {
            delete q;
        }
    }
// IELTS Listening Practice
class IELTSQuestion {
public:
    string audioText;  // Simulating audio with text for now
    string question;
    string correctAnswer;
    vector<string> options;

    IELTSQuestion(string text, string q, string ans, vector<string> opts)
        : audioText(text), question(q), correctAnswer(ans), options(opts) {}

    bool practice() {
        cout << "\n=== IELTS Listening Practice ===\n\n";
        cout << "Listen carefully (simulated audio):\n" << audioText << endl;
        cout << "\nQuestion: " << question << endl;

        for(size_t i = 0; i < options.size(); i++) {
            cout << char('A' + i) << ") " << options[i] << endl;
        }

        time_t start = time(nullptr);
        cout << "\nYour answer (A/B/C/D): ";
        string answer;
        getline(cin, answer);
        time_t end = time(nullptr);

        if(end - start > 60) {
            cout << "\nTime's up! The correct answer was: " << correctAnswer << endl;
            return false;
        }

        return (toupper(answer[0]) == correctAnswer[0]);
    }
};

// Quiz Card Game
class QuizCard {
public:
    string question;
    string answer;
    vector<string> options;

    QuizCard(string q, string ans, vector<string> opts)
        : question(q), answer(ans), options(opts) {}

    bool play() {
        cout << "\nQuestion: " << question << endl;
        for(size_t i = 0; i < options.size(); i++) {
            cout << i + 1 << ". " << options[i] << endl;
        }

        cout << "Your answer (1-" << options.size() << "): ";
        string response;
        getline(cin, response);

        if(isdigit(response[0])) {
            int choice = stoi(response) - 1;
            if(choice >= 0 && choice < options.size()) {
                return options[choice] == answer;
            }
        }
        return false;
    }
};

vector<IELTSQuestion> initializeIELTSQuestions() {
    vector<IELTSQuestion> questions;

    questions.push_back(IELTSQuestion(
        "The university library will be closed for renovation from July 1st to August 15th. "
        "During this period, students can access the online resources and use the temporary "
        "study space in Building B.",
        "When will the library reopen?",
        "August 15th",
        {"July 1st", "August 1st", "August 15th", "September 1st"}
    ));

    // Add more questions as needed
    return questions;
}

vector<QuizCard> initializeQuizCards() {
    vector<QuizCard> cards;

    cards.push_back(QuizCard(
        "What is the correct meaning of 'ubiquitous'?",
        "Present everywhere",
        {"Present everywhere", "Very rare", "Unusual", "Unique"}
    ));

    // Add more cards as needed
    return cards;
}

void practiceIELTS(vector<IELTSQuestion>& questions) {
    cout << "\n=== IELTS Listening Practice ===\n";
    int score = 0;

    for(auto& question : questions) {
        if(question.practice()) {
            cout << "\nCorrect!" << endl;
            score++;
        } else {
            cout << "\nIncorrect." << endl;
        }
        cout << "\nPress Enter to continue...";
        cin.get();
    }

    cout << "\nFinal Score: " << score << "/" << questions.size() << endl;
}

void playQuizGame(vector<QuizCard>& cards) {
    cout << "\n=== English Quiz Game ===\n";
    int score = 0;

    for(auto& card : cards) {
        if(card.play()) {
            cout << "\nCorrect!" << endl;
            score++;
        } else {
            cout << "\nIncorrect. The correct answer was: " << card.answer << endl;
        }
        cout << "\nPress Enter to continue...";
        cin.get();
    }

    cout << "\nFinal Score: " << score << "/" << cards.size() << endl;
}

vector<IELTSQuestion> ieltsQuestions = initializeIELTSQuestions();
vector<QuizCard> quizCards = initializeQuizCards();

void premiumMenu() {
    system("cls");
    cout << "\n=== Premium Access ===\n";
    cout << "Please give 5 stars to both developers to continue.\n";
    cout << "Enter the secret code: ";

    string code;
    char ch;
    while((ch = _getch()) != '\r') {
        code += ch;
        cout << "*";
    }
    cout << endl;

    if(code == "access786") {
        cout << "\nPremium access granted!" << endl;

        while(true) {
            system("cls");
            cout << "\n=== Welcome to Language Learning App\n";
            cout << "1. Speak with ME\n";
            cout << "2. Listen and Practice\n";
            cout << "3. Mistakes\n";
            cout << "4. IELTS Listening Practice\n";
            cout << "5. Quiz Cards Game\n";
            cout << "6. Exit\n";
            cout << "Choose an option: ";

            string choice;
            getline(cin, choice);

            if(choice == "1") {
                speakWithMe();
            }
            else if(choice == "2") {
                listenAndPractice();
            }
            else if(choice == "3") {
                reviewMistakes();
            }
            else if(choice == "4") {
                practiceIELTS(ieltsQuestions);
            }
            else if(choice == "5") {
                playQuizGame(quizCards);
            }
            else if(choice == "6") break;

            if(choice != "7") {
                cout << "\nPress Enter to continue...";
                cin.get();
            }
        }
    } else {
        cout << "\nInvalid code!" << endl;
    }
}

    void displayMainMenu() {
        while (true) {
            system("cls"); // Clear screen
            cout << "\n=== Welcome to Language Learning App, " << userName << "! ===\n";
            cout << "1. Speak with ME\n";
            cout << "2. Listen and Practice\n";
            cout << "3. Mistakes\n";
            cout << "4. View Progress\n";
            cout << "5. Premium Menu\n";
            cout << "6. Exit\n";
            cout << "Choose an option: ";

            int choice;
            cin >> choice;
            cin.ignore();
            system("cls");
            switch (choice) {
                case 1: speakWithMe(); break;
                case 2: listenAndPractice(); break;
                case 3: reviewMistakes(); break;
                case 4: viewProgress(); break;
                case 5: premiumMenu(); break;
                case 6:
                    cout << "\nThank you for learning with us, " << userName << "!\n";
                    return;
                default:
                    cout << "Invalid choice! Press Enter to continue...";
                    cin.get();
            }
        }
    }

    void speakWithMe() {
        while (true) {
            system("cls");
            cout << "\n=== Categories ===\n";
            for (int i = 0; i < categories.size(); i++) {
                cout << i + 1 << ". " << categories[i].name << endl;
            }
            cout << "0. Back to Main Menu\n";
            cout << "Choose a category: ";

            int choice;
            cin >> choice;
            cin.ignore();
            system("cls");

            if (choice == 0) break;
            if (choice > 0 && choice <= categories.size()) {
                categories[choice - 1].displayWords();
                cout << "\nPress Enter to continue...";
                cin.get();
                system("cls");
            }
        }
    }

    void listenAndPractice() {
        // Show stories
        for (const Story& story : stories) {
            system("cls");
            story.display();
            cout << "\nPress Enter to continue...";
            cin.get();
            system("cls");
        }

        // Ask if user wants to take quiz
        cout << "\nWould you like to take the quiz? (Y/N): ";
        char choice;
        cin >> choice;
        cin.ignore();

        if (toupper(choice) == 'Y') {
            takeQuiz();
        }
    }

    void takeQuiz() {
        score = 0;
        system("cls");
        cout << "\n=== Quiz Time ===\n";

        for (int i = 0; i < allQuestions.size(); i++) {
            Ques* q = allQuestions[i];
            q->display(i + 1);

            cout << "Your answer (A/B/C/D): ";
            char answer;
            cin >> answer;
            cin.ignore();

            if (q->checkAnswer(answer)) {
                cout << "Correct!\n";
                score++;

            } else {
                cout << "Incorrect.\n";
                q->showCorrectAnswer();
                mistakeQueue.push(q);

            }
            cout << "\nPress Enter to continue...";
            cin.get();
            system("cls");
        }

        cout << "\nQuiz completed! Your score: " << score << "/" << allQuestions.size() << endl;
        cout << "Press Enter to continue...";
        cin.get();
        system("cls");
    }

    void reviewMistakes() {
        if (mistakeQueue.empty()) {
            cout << "\nNo mistakes to review!\n";
            cout << "Press Enter to continue...";
            cin.get();
            return;
        }

        cout << "\n=== Reviewing Mistakes ===\n";

        vector<Ques*> remainingMistakes;
        while (!mistakeQueue.empty()) {
            Ques* q = mistakeQueue.front();
            mistakeQueue.pop();

            q->display(1);
            cout << "Your answer (A/B/C/D): ";
            char answer;
            cin >> answer;
            cin.ignore();

            if (q->checkAnswer(answer)) {
                cout << "Correct!\n";
            } else {
                cout << "Still incorrect.\n";
                q->showCorrectAnswer();
                remainingMistakes.push_back(q);
            }
            cout << "\nPress Enter to continue...";
            cin.get();
            system("cls");
        }

        // Put remaining mistakes back in queue
        for (Ques* q : remainingMistakes) {
            mistakeQueue.push(q);
        }

        if (mistakeQueue.empty()) {
            cout << "\nCongratulations! You've corrected all your mistakes!\n";
        } else {
            cout << "\nKeep practicing! You have " << mistakeQueue.size()
                 << " mistakes left to review.\n";
        }
        cout << "Press Enter to continue...";
        cin.get();
    }

    void viewProgress() {
        system("cls");
        cout << "\n=== Progress Report for " << userName << " ===\n";
        cout << "Quiz Score: " << score << "/" << allQuestions.size() << endl;
        cout << "Mistakes to Review: " << mistakeQueue.size() << endl;

        cout << "\nPress Enter to continue...";
        cin.get();
    }
};

int main() {
    try {
        SetConsoleOutputCP(CP_UTF8);
        
        displayLogo();
        gotoRowCol(15, 30);
        cout << "1. Get Started\n";
        gotoRowCol(16, 30);
        cout << "2. I Already Have an Account\n";
        gotoRowCol(18, 30);
        cout << "Enter your choice: ";

        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            // New user path
            LeximoApp app;
            app.run();
        } 
        else if (choice == 2) {
            // Existing user path
            User user;
            string username, password;
            bool validUsername = false;
            UserManager userManager;

            while (!validUsername) {
                clearScreen();
                displayLogo();
                gotoRowCol(15, 25);
                cout << "Username: ";
                cin >> username;

                if (!userManager.usernameExists(username)) {
                    gotoRowCol(17, 25);
                    cout << "Username does not exist!\n";
                    Sleep(1500);
                    continue;
                }
                validUsername = true;
            }

            bool loginSuccess = false;
            do {
                clearScreen();
                displayLogo();
                gotoRowCol(15, 25);
                cout << "Username: " << username << "\n";
                gotoRowCol(16, 25);
                cout << "Password: ";
                cin >> password;

                if (userManager.verifyLogin(username, password)) {
                    loginSuccess = true;
                    user.username = username;
                    user.password = password;
                } else {
                    gotoRowCol(18, 25);
                    cout << "Incorrect password!\n";
                    Sleep(2000);
                }
            } while (!loginSuccess);

            clearScreen();
            displayLogo();
            gotoRowCol(15, 25);
            cout << "Welcome back, " << username << "!\n";
            Sleep(2000);

            // After successful login, start LanguageLearningApp
            LanguageLearningApp app;
            app.displayMainMenu();
        }

        return 0;

    } catch (const exception& e) {
        cerr << "An error occurred: " << e.what() << endl;
        return 1;
    }
}
