// @ts-check
import { defineConfig } from "astro/config";
import partytown from "@astrojs/partytown";
import sitemap from "@astrojs/sitemap";

// https://astro.build/config
export default defineConfig({
  site: "https://m88.computer",
  integrations: [
    partytown({
      config: {
        forward: ["dataLayer.push"],
      },
    }),
    sitemap(),
  ],
});
