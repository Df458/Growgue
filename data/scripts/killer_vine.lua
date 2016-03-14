function step()
    game.log("warning", "You carelessly step on the Killer Vine.");
    game.log("warning", "It makes a strange gesture at you.");
    game.log("fatal", "You feel like you're about to die!");
    player.hp = 1;
end
