function step()
    game.log("warning", "As you pass the " .. this.name .. ", it performs exactly as advertised.");
    player.hp = (player.hp - (player.hp_max / 3));
end
