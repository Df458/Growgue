function create()
    -- game.log("warning", "TEST LUA MESSAGE :D");
    -- if game.ask("default", "Say hello to the friendly " .. this.name .. "?") == true then
    --     game.log("default", "Hi!");
    -- else
    --     game.log("warning", "You aren't a nice person. The " .. this.name .. " doesn't like the cut of your jib.");
    --     this.aggro = 100;
    --     this.name = "Pissed-off " .. this.name;
    --     this.str = this.str * 5;
    -- end
end

function kill()
    game.log("error", "Bye now!");
end
