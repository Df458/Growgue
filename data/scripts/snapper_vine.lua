function step()
    game.log("warning", "You carelessly step on the Snapper Vine.");
    game.log("warning", "*SNAP* It bites you!");
    player.hp = player.hp - (player.hp_max / 5);
end
