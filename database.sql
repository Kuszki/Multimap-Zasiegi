SET FOREIGN_KEY_CHECKS=0;
SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
CREATE DATABASE IF NOT EXISTS `zasiegi` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;
USE `zasiegi`;

DROP TABLE IF EXISTS `blokady`;
CREATE TABLE `blokady` (
  `id` int(10) UNSIGNED NOT NULL,
  `operator` varchar(32) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `dokumenty`;
CREATE TABLE `dokumenty` (
  `id` int(10) UNSIGNED NOT NULL,
  `nazwa` varchar(128) NOT NULL,
  `operat` int(10) UNSIGNED NOT NULL,
  `rodzaj` int(10) UNSIGNED DEFAULT NULL,
  `sciezka` varchar(512) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `gminy`;
CREATE TABLE `gminy` (
  `id` int(10) UNSIGNED NOT NULL,
  `nazwa` varchar(64) NOT NULL,
  `kod` varchar(32) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `obreby`;
CREATE TABLE `obreby` (
  `id` int(10) UNSIGNED NOT NULL,
  `gmina` int(10) UNSIGNED NOT NULL,
  `nazwa` varchar(64) NOT NULL,
  `kod` varchar(32) NOT NULL,
  `numer` int(10) UNSIGNED NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `operaty`;
CREATE TABLE `operaty` (
  `id` int(10) UNSIGNED NOT NULL,
  `numer` varchar(32) NOT NULL,
  `rodzaj` int(10) UNSIGNED DEFAULT NULL,
  `data` datetime DEFAULT NULL,
  `operator` varchar(64) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `rodzajedok`;
CREATE TABLE `rodzajedok` (
  `id` int(10) UNSIGNED NOT NULL,
  `nazwa` varchar(128) NOT NULL,
  `symbol` varchar(32) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `rodzajeopr`;
CREATE TABLE `rodzajeopr` (
  `id` int(10) UNSIGNED NOT NULL,
  `nazwa` varchar(128) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `zmiany`;
CREATE TABLE `zmiany` (
  `id` int(10) UNSIGNED NOT NULL,
  `dokument` int(10) UNSIGNED NOT NULL,
  `arkusz` varchar(32) NOT NULL,
  `obreb` int(10) UNSIGNED NOT NULL,
  `stare` varchar(1024) NOT NULL,
  `nowe` varchar(1024) NOT NULL,
  `uwagi` text DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


ALTER TABLE `blokady`
  ADD PRIMARY KEY (`id`);

ALTER TABLE `dokumenty`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `SCIEZKA` (`sciezka`),
  ADD KEY `RODZAJ` (`rodzaj`),
  ADD KEY `OPERAT` (`operat`) USING BTREE;

ALTER TABLE `gminy`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `NAZWA` (`nazwa`),
  ADD UNIQUE KEY `KOD` (`kod`);

ALTER TABLE `obreby`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `KOD` (`kod`),
  ADD UNIQUE KEY `NAZWA` (`nazwa`,`gmina`) USING BTREE,
  ADD UNIQUE KEY `NUMER` (`numer`,`gmina`),
  ADD KEY `GMINA` (`gmina`);

ALTER TABLE `operaty`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `NUMER` (`numer`),
  ADD KEY `RODZAJ` (`rodzaj`);

ALTER TABLE `rodzajedok`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `NAZWA` (`nazwa`);

ALTER TABLE `rodzajeopr`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `NAZWA` (`nazwa`);

ALTER TABLE `zmiany`
  ADD PRIMARY KEY (`id`),
  ADD KEY `DOKUMENT` (`dokument`),
  ADD KEY `OBREB` (`obreb`);


ALTER TABLE `dokumenty`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;

ALTER TABLE `gminy`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;

ALTER TABLE `obreby`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;

ALTER TABLE `operaty`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;

ALTER TABLE `rodzajedok`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;

ALTER TABLE `rodzajeopr`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;

ALTER TABLE `zmiany`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;


ALTER TABLE `blokady`
  ADD CONSTRAINT `ID` FOREIGN KEY (`id`) REFERENCES `operaty` (`id`) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE `dokumenty`
  ADD CONSTRAINT `OPERAT` FOREIGN KEY (`operat`) REFERENCES `operaty` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `RODZAJD` FOREIGN KEY (`rodzaj`) REFERENCES `rodzajedok` (`id`);

ALTER TABLE `obreby`
  ADD CONSTRAINT `GMINA` FOREIGN KEY (`gmina`) REFERENCES `gminy` (`id`) ON DELETE CASCADE ON UPDATE CASCADE;

ALTER TABLE `operaty`
  ADD CONSTRAINT `RODZAJO` FOREIGN KEY (`rodzaj`) REFERENCES `rodzajeopr` (`id`);

ALTER TABLE `zmiany`
  ADD CONSTRAINT `DOKUMENT` FOREIGN KEY (`dokument`) REFERENCES `operaty` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `OBREB` FOREIGN KEY (`obreb`) REFERENCES `obreby` (`id`);
SET FOREIGN_KEY_CHECKS=1;
COMMIT;
