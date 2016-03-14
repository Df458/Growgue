function attacked()
    v = math.random(3);
    if v == 1 then
        game.log("default", "The crown helps you!");
        player.hp = player.hp + 10;
    elseif v == 2 then
        game.log("default", "The crown helps you!");
        player.hp = player.hp + 25;
    else
        game.log("default", "The crown hinders you!");
        player.hp = player.hp - 10;
    end
end
