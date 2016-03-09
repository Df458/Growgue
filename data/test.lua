function create()
    game.log("warning", "TEST LUA MESSAGE :D");
    if game.ask("default", "Say hello?") == true then
        game.log("default", "Hi!");
    else
        game.log("warning", "You aren't a nice person");
    end
end

function kill()
    game.log("error", "Bye now!");
end
