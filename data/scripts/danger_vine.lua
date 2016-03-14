function step()
    game.log("warning", "You carelessly step on the Danger Vine.");
    game.log("warning", "It strangles you!");
    player.hp = player.hp - (player.hp_max / 2);
end
