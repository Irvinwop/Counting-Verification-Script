# A Counting Verification Script
A counting script written in some questionable C++ which should determine if all the values in a channel (excluding MEE6) are valid, ascending incrementally counts.
Then, it should send the results of its count over to a channel of your choosing.
To run,
1. Create an info.json in the template of info_template.json
2. Compile the C++ program and run it, or go to (3) for automatic running on a machine.
3. Ensure Docker is running
4. Run `docker-compose build uwu`
5. Use systemd or some other task scheduler to run `docker-compose up uwu` at intervals of your choosing.
6. Overwrite `runtime/send_leaderboard.flag` as a non-zero integer when you wish to send a leaderboard to your status channel.
