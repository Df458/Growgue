function dropped()
    game.log("warning", "You MONSTER. You have abandoned your child. Do you hate onions?");
    game.log("warning", "The onion starts to cry.");
    game.log("warning", "The crying is making you tired. The onion changes from its' natural " .. this.color .. " hue!");
    this.color = "EP critical";
    player.ep = 1;
end

function picked_up()
    game.log("warning", "The onion won't return to its mother; it's your responsibility now.");
end

function step()
    game.log("warning", "You accidentally step on an onion, crushing it.");
    game.log("warning", "You start to cry.");
    game.log("warning", "You have " .. tostring(player.ep) .. " EP.");
    -- this.kill();
end
