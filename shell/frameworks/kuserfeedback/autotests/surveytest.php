<?php
/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

require_once('datastoretesthelper.php');
require_once('../src/server/shared/product.php');
require_once('../src/server/shared/survey.php');

class SurveyTest extends PHPUnit\Framework\TestCase
{
    private static $db;

    public static function setUpBeforeClass(): void
    {
        self::$db = DatastoreTestHelper::setup();
    }

    public function testFromJson()
    {
        $s = Survey::fromJson('{
            "name": "mySurvey",
            "url": "http://survey.example/",
            "active": true,
            "target": "screen[0].dpi > 100",
            "uuid": "{9e529dfa-0213-413e-a1a8-8a9cea7d5a97}"
        }');
        $this->assertEquals($s->name, 'mySurvey');
        $this->assertEquals($s->url, 'http://survey.example/');
        $this->assertEquals($s->active, true);
        $this->assertEquals($s->uuid, '{9e529dfa-0213-413e-a1a8-8a9cea7d5a97}');
        $this->assertEquals($s->target, 'screen[0].dpi > 100');
    }

    public function testToJson()
    {
        $s = new Survey();
        $s->name = 'mySurvey';
        $s->url = 'http://survey.example/';
        $s->active = true;
        $s->uuid = '{9e529dfa-0213-413e-a1a8-8a9cea7d5a97}';
        $s->target = 'screen[0].dpi > 100';

        $this->assertJsonStringEqualsJsonString(json_encode($s), '{
            "uuid": "{9e529dfa-0213-413e-a1a8-8a9cea7d5a97}",
            "name": "mySurvey",
            "url": "http://survey.example/",
            "active": true,
            "target": "screen[0].dpi > 100"
        }');
    }

    public function testGetSurveys()
    {
        $surveys = Survey::surveysForProduct(self::$db, 'I don\'t exist');
        $this->assertCount(0, $surveys);

        $surveys = Survey::surveysForProduct(self::$db, 'org.kde.UnitTest');
        $this->assertCount(2, $surveys);

        $survey1 = $surveys[0];
        $this->assertInstanceOf(Survey::class, $survey1);
        $survey2 = $surveys[1];
        $this->assertInstanceOf(Survey::class, $survey2);

        if ($survey1->name > $survey2->name) {
            $survey1 = $surveys->schema[1];
            $survey2 = $surveys->schema[0];
        }

        $this->assertEquals($survey1->name, 'myActiveSurvey');
        $this->assertEquals($survey1->url, 'http://survey.example/active');
        $this->assertEquals($survey1->active, true);
        $this->assertEquals($survey1->target, 'usageTime.value >= 3600');

        $this->assertEquals($survey2->name, 'myInactiveSurvey');
        $this->assertEquals($survey2->url, 'http://survey.example/inactive');
        $this->assertEquals($survey2->active, false);
        $this->assertEquals($survey2->target, 'screen[0].dpi < 200');

        $p = Product::productByName(self::$db, 'org.kde.UnitTest');
        $this->assertNotNull($p);
        $surveys = Survey::activeSurveysForProduct(self::$db, $p);
        $this->assertCount(1, $surveys);

        $s = $surveys[0];
        $this->assertInstanceOf(Survey::class, $s);
        $this->assertEquals($s->name, 'myActiveSurvey');
        $this->assertEquals($s->url, 'http://survey.example/active');
        $this->assertEquals($s->active, true);
        $this->assertEquals($s->target, 'usageTime.value >= 3600');
    }

    public function testSurveyInsert()
    {
        $json = '{
            "name": "newSurvey",
            "uuid": "{9e529dfa-0213-413e-a1a8-8a9cea7d5a97}",
            "url": "http://survey.example/new",
            "active": true,
            "target": "startCount.value > 10"
        }';

        $p = Product::productByName(self::$db, 'org.kde.UnitTest');
        $this->assertNotNull($p);

        $s = Survey::fromJson($json);
        $s->insert(self::$db, $p);

        $surveys = Survey::activeSurveysForProduct(self::$db, $p);
        $s = null;
        foreach ($surveys as $survey) {
            if ($survey->name == 'newSurvey') {
                $s = $survey;
                break;
            }
        }
        $this->assertNotNull($s);
        $this->assertEquals($s->name, 'newSurvey');
        $this->assertEquals($s->url, 'http://survey.example/new');
        $this->assertEquals($s->active, true);
        $this->assertEquals($s->target, 'startCount.value > 10');
        $this->assertEquals($s->uuid, '{9e529dfa-0213-413e-a1a8-8a9cea7d5a97}');
    }

    public function testSurveyUpdate()
    {
        $json = '{
            "name": "myChangedSurvey",
            "url": "http://survey.example/changed",
            "active": false,
            "uuid": "{962bbd80-7120-4f18-a4c0-5800fa323868}",
            "target": "views[\"editor\"].ratio > 0.5"
        }';

        $s = Survey::fromJson($json);
        $s->update(self::$db);

        $surveys = Survey::surveysForProduct(self::$db, 'org.kde.UnitTest');
        $s = null;
        foreach ($surveys as $survey) {
            if ($survey->name == 'myChangedSurvey') {
                $s = $survey;
                break;
            }
        }
        $this->assertNotNull($s);
        $this->assertEquals($s->name, 'myChangedSurvey');
        $this->assertEquals($s->url, 'http://survey.example/changed');
        $this->assertEquals($s->active, false);
        $this->assertEquals($s->target, 'views["editor"].ratio > 0.5');
    }

    public function testSurveyDelete()
    {
        $json = '{
            "name": "myInactiveSurvey",
            "url": "http://survey.example/inactive",
            "active": false,
            "uuid": "{bdfa82c5-238f-404b-a441-07ca3d3eff7f}"
        }';

        $s = Survey::fromJson($json);
        $s->delete(self::$db);

        $surveys = Survey::surveysForProduct(self::$db, 'org.kde.UnitTest');
        $s = null;
        foreach ($surveys as $survey) {
            if ($survey->name == 'myInactiveSurvey') {
                $s = $survey;
                break;
            }
        }
        $this->assertNull($s);
    }

    public static function invalidInput_data()
    {
        return [
            'empty' => [ '{}' ],
            'mising url' => [ '{ "name": "surveyName" }' ],
            'missing active' => [ '{ "name": "surveyName", "url": "http://survey.example/" }' ],
            'empty name' => [ '{ "name": "", "url": "http://survey.example/", "active": false }' ],
            'non-string name' => [ '{ "name": 123, "url": "http://survey.example/", "active": false }' ],
            'non-numberic id' => [ '{ "name": "myName", "url": "http://survey.example/", "active": true, "id": "string" }' ]
        ];
    }

    /**
     * @dataProvider invalidInput_data
     */
    public function testInvalidInput($json)
    {
        $this->expectException(RESTException::class);
        $this->expectExceptionCode(400);
        $s = Survey::fromJson($json);
    }
}


