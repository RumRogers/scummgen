#include "Game.hpp"
#include "util/IO.hpp"
#include "util/Log.hpp"
#include "util/XMLFile.hpp"
#include "grammar/Context.hpp"
#include "grammar/Declaration.hpp"
#include "grammar/Function.hpp"
#include "Room.hpp"
#include "Object.hpp"
#include "Costume.hpp"
#include "Charset.hpp"
#include "Voice.hpp"

// External flex/bison declarations
extern FILE *yyin;
extern int yyparse(vector<Declaration *> &declarations, vector<Function *> &functions);
extern int yylineno;

const uint8_t Game::N_DEFAULT_ACTORS = 13;
const uint16_t Game::MAX_WORD_VARIABLES = 8192;
const uint8_t Game::MAX_LOCAL_VARIABLES = 16;

Array::Array(XMLNode *node)
{
	Log::getInstance().write(LOG_INFO, "Array\n");
	Log::getInstance().indent();
	
	_varNumber = node->getChild("varNumber")->getIntegerContent();
	Log::getInstance().write(LOG_INFO, "varNumber: %u\n", _varNumber);

	_dimA = node->getChild("dimA")->getIntegerContent();
	Log::getInstance().write(LOG_INFO, "dimA: %u\n", _dimA);

	_dimB = node->getChild("dimB")->getIntegerContent();
	Log::getInstance().write(LOG_INFO, "dimB: %u\n", _dimB);
	
	_type = node->getChild("type")->getIntegerContent();
	Log::getInstance().write(LOG_INFO, "type: %u\n", _type);
	
	Log::getInstance().unIndent();
}

Array::~Array()
{
}

Game::Game(string dirName)
{
	Log::getInstance().write(LOG_INFO, "Game\n");
	Log::getInstance().indent();

	XMLFile xmlFile;
	if (!xmlFile.open(dirName + "game.xml"))
		Log::getInstance().write(LOG_ERROR, "Couldn't find any game in the specified directory !\n");
	XMLNode *node = xmlFile.getRootNode();

	_longName = node->getChild("longName")->getStringContent();
	Log::getInstance().write(LOG_INFO, "longName: %s\n", _longName.c_str());

	_shortName = node->getChild("shortName")->getStringContent();
	Log::getInstance().write(LOG_INFO, "shortName: %s\n", _shortName.c_str());

	_key = node->getChild("key")->getIntegerContent();
	Log::getInstance().write(LOG_INFO, "key: 0x%02x\n", _key);
	
	int i = 0;
	XMLNode *child;
	while ((child = node->getChild("array", i++)) != NULL)
		_arrays.push_back(new Array(child));

	loadRooms(dirName + "rooms/");
	loadObjects(dirName + "objects/");
	loadScripts(dirName + "scripts/");
	loadCostumes(dirName + "costumes/");
	loadCharsets(dirName + "charsets/");
	loadVoices(dirName + "voices/");

	Log::getInstance().unIndent();
}

void Game::loadRooms(string dirName)
{
	XMLFile xmlFile;
	xmlFile.open(dirName + "rooms.xml");
	XMLNode *node = xmlFile.getRootNode();

	if (node == NULL)
	{
		Log::getInstance().write(LOG_WARNING, "Game does not contain any room !\n");
		return;
	}

	int i = 0;
	XMLNode *child;
	while ((child = node->getChild("room", i++)) != NULL)
		_rooms.push_back(new Room(dirName + child->getStringContent() + "/"));

	if (_rooms.empty())
		Log::getInstance().write(LOG_WARNING, "Game does not contain any room !\n");
}

void Game::loadObjects(string dirName)
{
	XMLFile xmlFile;
	xmlFile.open(dirName + "objects.xml");
	XMLNode *node = xmlFile.getRootNode();

	if (node == NULL)
		return;

	int i = 0;
	XMLNode *child;
	while ((child = node->getChild("object", i++)) != NULL)
		_objects.push_back(new Object(dirName + child->getStringContent() + "/"));
}

void Game::loadScripts(string dirName)
{
	XMLFile xmlFile;
	xmlFile.open(dirName + "scripts.xml");
	XMLNode *node = xmlFile.getRootNode();

	if (node == NULL)
	{
		Log::getInstance().write(LOG_WARNING, "Couldn't find any global script !\n");
		return;
	}

	int i = 0;
	XMLNode *child;
	while ((child = node->getChild("script", i++)) != NULL)
		_scripts.push_back(dirName + child->getStringContent() + ".sgc");

	if (_scripts.empty())
		Log::getInstance().write(LOG_WARNING, "Couldn't find any global script !\n");
}

void Game::loadCharsets(string dirName)
{
	XMLFile xmlFile;
	xmlFile.open(dirName + "charsets.xml");
	XMLNode *node = xmlFile.getRootNode();

	if (node == NULL)
	{
		Log::getInstance().write(LOG_WARNING, "Game does not contain any charset !\n");
		return;
	}

	int i = 0;
	XMLNode *child;
	while ((child = node->getChild("charset", i++)) != NULL)
		_charsets.push_back(new Charset(dirName + child->getStringContent() + "/"));

	if (_charsets.empty())
		Log::getInstance().write(LOG_WARNING, "Game does not contain any charset !\n");
}

void Game::loadCostumes(string dirName)
{
	XMLFile xmlFile;
	xmlFile.open(dirName + "costumes.xml");
	XMLNode *node = xmlFile.getRootNode();

	if (node == NULL)
	{
		Log::getInstance().write(LOG_WARNING, "Game does not contain any global costume !\n");
		return;
	}

	int i = 0;
	XMLNode *child;
	while ((child = node->getChild("costume", i++)) != NULL)
		_costumes.push_back(new Costume(dirName + child->getStringContent() + "/"));

	if (_charsets.empty())
		Log::getInstance().write(LOG_WARNING, "Game does not contain any global costume !\n");
}

void Game::loadVoices(string dirName)
{
	XMLFile xmlFile;
	xmlFile.open(dirName + "voices.xml");
	XMLNode *node = xmlFile.getRootNode();

	if (node == NULL)
	{
		Log::getInstance().write(LOG_WARNING, "Game does not contain any voice !\n");
		return;
	}

	int i = 0;
	XMLNode *child;
	while ((child = node->getChild("voice", i++)) != NULL)
		_voices.push_back(new Voice(dirName + child->getStringContent() + "/"));

	if (_voices.empty())
		Log::getInstance().write(LOG_WARNING, "Game does not contain any voice !\n");
}

void Game::parse()
{
	Log::getInstance().write(LOG_INFO, "Parsing game...\n");
	Log::getInstance().indent();

	if (_scripts.empty())
		Log::getInstance().write(LOG_ERROR, "Couldn't find any global script !\n");

	bool foundMain = false;
	uint16_t id = 2;

	for (int i = 0; i < _scripts.size(); i++)
	{
		Log::getInstance().write(LOG_INFO, "Parsing \"%s\"...\n", _scripts[i].c_str());
		Log::getInstance().indent();
		yyin = fopen(_scripts[i].c_str(), "r");
		vector<Function *> functions;
		yylineno = 1;
		if (yyparse(_declarations, functions))
		{
			fclose(yyin);
			for (int j = 0; j < functions.size(); j++)
				delete functions[j];
			Log::getInstance().write(LOG_ERROR, "Parsing error !\n");
		}
		for (int j = 0; j < functions.size(); j++)
			// Special case for the main function
			if (functions[j]->getName() == "main")
			{
				functions[j]->setID(1);
				_functions.insert(_functions.begin(), functions[j]);
				foundMain = true;
			}
			else
			{
				functions[j]->setID(id++);
				_functions.push_back(functions[j]);
			}
		fclose(yyin);
		Log::getInstance().unIndent();
	}

	if (!foundMain)
		Log::getInstance().write(LOG_ERROR, "Couldn't find the main function !\n");

	for (int i = 0; i < _rooms.size(); i++)
		_rooms[i]->parse(_declarations);

	Log::getInstance().unIndent();
}

void Game::compile()
{
	Log::getInstance().write(LOG_INFO, "Compiling game scripts...\n");
	Log::getInstance().indent();

	Context context(CONTEXT_GAME, &_declarations, &_functions, -1, -1, -1);
	Context::pushContext(&context);

	// We compile the common global functions
	for (int i = 0; i < _functions.size(); i++)
		_functions[i]->compile();

	// Then we compile the room functions
	for (int i = 0; i < _rooms.size(); i++)
		_rooms[i]->compile();

	Context::popContext();

	Log::getInstance().unIndent();
}

Game::~Game()
{
	for (int i = 0; i < _arrays.size(); i++)
		delete _arrays[i];
	for (int i = 0; i < _rooms.size(); i++)
		delete _rooms[i];
	for (int i = 0; i < _costumes.size(); i++)
		delete _costumes[i];
	for (int i = 0; i < _charsets.size(); i++)
		delete _charsets[i];
	for (int i = 0; i < _voices.size(); i++)
		delete _voices[i];
	for (int i = 0; i < _declarations.size(); i++)
		delete _declarations[i];
	for (int i = 0; i < _functions.size(); i++)
		delete _functions[i];
}
