javabot="java -jar /Users/keithm/tmp/google-ai-ants-wsf-read-only/GoogleAIAnts/src/MyBot.jar"

shylux="java -jar /Users/keithm/tmp/shylux-ant-bot-read-only/shylux.jar"

malazan="/Users/keithm/tmp/malazan/malazan-ants-read-only/src/malazan"

umbc="java -jar /Users/keithm/tmp/umbc/umbc-ants/AntsImplementation/dist/aichallengeKWB.jar"

hunter="python tools/sample_bots/python/HunterBot.py"


#python tools/playgame.py "./MyBot" "${hunter}" --map_file tools/maps/example/tutorial1.map --log_dir game_logs --turns 200 --scenario --food none --player_seed 7 --verbose -e --turntime 5000

#python tools/playgame.py "./MyBot" "python tools/sample_bots/python/HunterBot.py" --map_file tools/maps/random_walk/random_walk_02p_01.map --log_dir game_logs --turns 300  --player_seed 7 --verbose -e

#python tools/playgame.py "./MyBot" "python tools/sample_bots/python/HunterBot.py"  "python tools/sample_bots/python/HunterBot.py" "python tools/sample_bots/python/HunterBot.py" --map_file tools/maps/multi_hill_maze/maze_04p_01.map --log_dir game_logs --turns 300  --player_seed 7 --verbose -e

#python tools/playgame.py "./MyBot" "${malazan}" --map_file tools/maps/random_walk/random_walk_02p_01.map --log_dir game_logs --turns 300  --player_seed 7 --verbose -e

#python tools/playgame.py "./MyBot" "${umbc}" --map_file tools/maps/random_walk/random_walk_02p_01.map --log_dir game_logs --turns 300  --player_seed 7 --verbose -e

#python tools/playgame.py "./MyBot" "${javabot}" --map_file tools/maps/random_walk/random_walk_02p_01.map --log_dir game_logs --turns 300  --player_seed 7 --verbose -e

#python tools/playgame.py "./MyBot" "${shylux}" --map_file tools/maps/random_walk/random_walk_02p_01.map --log_dir game_logs --turns 300  --player_seed 7 --verbose -e

#python tools/playgame.py "./MyBot" "${shylux}" "${malazan}" "${umbc}" --map_file tools/maps/multi_hill_maze/maze_04p_01.map --log_dir game_logs --turns 1000  --player_seed 7 --verbose -e


#python tools/playgame.py "./MyBot" "${shylux}"  "${umbc}" "${malazan}" --map_file tools/maps/multi_hill_maze/maze_04p_02.map --log_dir game_logs --turns 500  --player_seed 7 --verbose -e

python tools/playgame.py "./MyBot" "${shylux}"  "${umbc}" "${malazan}" --map_file tools/maps/cell_maze/cell_maze_p04_04.map --log_dir game_logs --turns 500  --player_seed 7 --verbose -e
